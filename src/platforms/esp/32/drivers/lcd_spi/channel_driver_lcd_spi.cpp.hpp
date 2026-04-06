// IWYU pragma: private

/// @file channel_driver_lcd_spi.cpp
/// @brief LCD_CAM SPI channel driver implementation

#include "platforms/is_platform.h"

// Compile for ESP32-S3 (with LCD_CAM) or host testing
#if defined(FASTLED_STUB_IMPL) || \
    (!defined(ARDUINO) &&          \
     (defined(__linux__) || defined(__APPLE__) || defined(_WIN32)))
#define FL_LCD_SPI_COMPILE 1
#elif defined(FL_IS_ESP32)
#include "sdkconfig.h"
#include "platforms/esp/32/feature_flags/enabled.h"
#if FASTLED_ESP32_HAS_LCD_SPI
#define FL_LCD_SPI_COMPILE 1
#endif
#endif

#ifdef FL_LCD_SPI_COMPILE

#include "platforms/esp/32/drivers/lcd_spi/channel_driver_lcd_spi.h"
#include "fl/system/log.h"
#include "fl/task/executor.h"
#include "fl/stl/cstring.h"
#include "fl/stl/noexcept.h"

// Include ESP peripheral for factory function
#if defined(FL_IS_ESP_32S3)
#include "fl/stl/has_include.h"
#if FL_HAS_INCLUDE("esp_lcd_panel_io.h")
// IWYU pragma: begin_keep
#include "platforms/esp/32/drivers/lcd_spi/lcd_spi_peripheral_esp.h"
// IWYU pragma: end_keep
#define FL_LCD_SPI_HAS_ESP_PERIPHERAL 1
#endif
#endif
#ifndef FL_LCD_SPI_HAS_ESP_PERIPHERAL
#define FL_LCD_SPI_HAS_ESP_PERIPHERAL 0
#endif

namespace fl {

ChannelDriverLcdSpi::ChannelDriverLcdSpi(
    fl::shared_ptr<detail::ILcdSpiPeripheral> peripheral) FL_NOEXCEPT
    : mPeripheral(fl::move(peripheral)), mInitialized(false),
      mEnqueuedChannels(), mTransmittingChannels(), mBuffer(nullptr),
      mBufferSize(0), mBusy(false) {}

ChannelDriverLcdSpi::~ChannelDriverLcdSpi() {
    // Block until DMA completes — mBuffer is the DMA source for LCD_CAM,
    // so freeing it while DMA is active would be a use-after-free.
    if (mBusy && mPeripheral) {
        mPeripheral->waitTransmitDone(2000);
        mBusy = false;
    }
    if (mPeripheral && mBuffer != nullptr) {
        mPeripheral->freeBuffer(mBuffer);
        mBuffer = nullptr;
    }
}

bool ChannelDriverLcdSpi::canHandle(
    const ChannelDataPtr &data) const FL_NOEXCEPT {
    if (!data) {
        return false;
    }
    return data->isSpi();
}

void ChannelDriverLcdSpi::enqueue(ChannelDataPtr channelData) FL_NOEXCEPT {
    mEnqueuedChannels.push_back(fl::move(channelData));
}

void ChannelDriverLcdSpi::show() FL_NOEXCEPT {
    // Release channels from any completed previous transmission
    poll();

    if (mEnqueuedChannels.empty()) {
        return;
    }

    // Wait for previous transmission to complete (max ~5 seconds)
    constexpr int kMaxIterations = 20000;
    int iterations = 0;
    while (mBusy && iterations++ < kMaxIterations) {
        poll();
        fl::task::run(250, fl::task::ExecFlags::SYSTEM);
    }
    if (mBusy) {
        FL_WARN("ChannelDriverLcdSpi: DMA hung — forcing release");
        mBusy = false;
        for (auto &channel : mTransmittingChannels) {
            channel->setInUse(false);
        }
        mTransmittingChannels.clear();
    }

    mTransmittingChannels = fl::move(mEnqueuedChannels);

    if (!beginTransmission(mTransmittingChannels)) {
        for (auto &channel : mTransmittingChannels) {
            channel->setInUse(false);
        }
        mTransmittingChannels.clear();
    }
}

IChannelDriver::DriverState ChannelDriverLcdSpi::poll() FL_NOEXCEPT {
    if (mTransmittingChannels.empty()) {
        return DriverState::READY;
    }

    if (mPeripheral && !mPeripheral->isBusy()) {
        mBusy = false;
        for (auto &channel : mTransmittingChannels) {
            channel->setInUse(false);
        }
        mTransmittingChannels.clear();
        return DriverState::READY;
    }

    return mBusy ? DriverState::DRAINING : DriverState::READY;
}

void ChannelDriverLcdSpi::transposeToWords(
    fl::span<const ChannelDataPtr> channels, u16 *output,
    size_t maxBytes) FL_NOEXCEPT {
    // For each byte position, create a 16-bit word where each bit
    // corresponds to a data lane. Bit N of the word = bit 7 (MSB first)
    // of the byte from lane N at this position.
    //
    // Each input byte produces 8 output words (one per bit, MSB first).
    for (size_t byteIdx = 0; byteIdx < maxBytes; byteIdx++) {
        // Gather this byte from all lanes
        u8 lane_bytes[16];
        fl::memset(lane_bytes, 0, sizeof(lane_bytes));

        for (size_t lane = 0; lane < channels.size() && lane < 16; lane++) {
            const auto &data = channels[lane]->getData();
            if (byteIdx < data.size()) {
                lane_bytes[lane] = data[byteIdx];
            }
        }

        // Transpose: 8 bits per byte -> 8 words
        for (int bit = 7; bit >= 0; bit--) {
            u16 word = 0;
            for (size_t lane = 0; lane < channels.size() && lane < 16;
                 lane++) {
                if (lane_bytes[lane] & (1 << bit)) {
                    word |= (1 << lane);
                }
            }
            *output++ = word;
        }
    }
}

bool ChannelDriverLcdSpi::beginTransmission(
    fl::span<const ChannelDataPtr> channels) FL_NOEXCEPT {
    if (channels.empty() || !mPeripheral) {
        return false;
    }

    size_t maxSize = 0;
    for (const auto &channel : channels) {
        size_t sz = channel->getSize();
        if (sz > maxSize) {
            maxSize = sz;
        }
    }

    if (maxSize == 0) {
        return false;
    }

    int numLanes = static_cast<int>(channels.size());

    // Each byte produces 8 u16 words (one per bit, transposed across lanes)
    size_t totalWords = maxSize * 8;
    size_t bufferBytes = totalWords * sizeof(u16);

    if (!mInitialized || mBufferSize < bufferBytes) {
        if (mBuffer != nullptr) {
            mPeripheral->freeBuffer(mBuffer);
            mBuffer = nullptr;
        }

        if (mInitialized) {
            mPeripheral->deinitialize();
            mInitialized = false;
        }

        detail::LcdSpiConfig config;
        config.num_lanes = numLanes;
        config.max_transfer_bytes = bufferBytes;
        config.use_psram = true;

        for (size_t i = 0; i < channels.size() && i < 16; i++) {
            const auto &chipset = channels[i]->getChipset();
            const auto *spiConfig = chipset.ptr<SpiChipsetConfig>();
            if (spiConfig) {
                config.data_gpios[i] = spiConfig->dataPin;
                if (config.clock_gpio < 0) {
                    config.clock_gpio = spiConfig->clockPin;
                    config.clock_hz = spiConfig->timing.clock_hz;
                }
            }
        }

        // DC pin is required by the I80 bus API but unused for LED data.
        // Default to GPIO 21 — a non-strapping pin on ESP32-S3.
        // Avoid GPIO 0 (boot mode) and GPIO 45/46 (VDD_SPI strapping).
        if (config.dc_gpio < 0) {
            config.dc_gpio = 21;
        }

        if (!mPeripheral->initialize(config)) {
            FL_WARN("ChannelDriverLcdSpi: Failed to initialize peripheral");
            return false;
        }

        mBuffer = mPeripheral->allocateBuffer(bufferBytes);
        if (mBuffer == nullptr) {
            FL_WARN("ChannelDriverLcdSpi: Failed to allocate buffer");
            return false;
        }
        mBufferSize = bufferBytes;
        mInitialized = true;
    }

    fl::memset(mBuffer, 0, bufferBytes);
    transposeToWords(channels, mBuffer, maxSize);

    for (const auto &channel : channels) {
        channel->setInUse(true);
    }

    mBusy = true;
    if (!mPeripheral->transmit(mBuffer, bufferBytes)) {
        mBusy = false;
        for (const auto &channel : channels) {
            channel->setInUse(false);
        }
        FL_WARN("ChannelDriverLcdSpi: Transmit failed");
        return false;
    }

    // show() returns immediately — poll() drives completion via DRAINING state.
    return true;
}

fl::shared_ptr<IChannelDriver> createLcdSpiEngine() FL_NOEXCEPT {
#if FL_LCD_SPI_HAS_ESP_PERIPHERAL
    class EspWrapper : public detail::ILcdSpiPeripheral {
      public:
        bool initialize(const detail::LcdSpiConfig &c) FL_NOEXCEPT override {
            return detail::LcdSpiPeripheralEsp::instance().initialize(c);
        }
        void deinitialize() FL_NOEXCEPT override {
            detail::LcdSpiPeripheralEsp::instance().deinitialize();
        }
        bool isInitialized() const FL_NOEXCEPT override {
            return detail::LcdSpiPeripheralEsp::instance().isInitialized();
        }
        u16 *allocateBuffer(size_t s) FL_NOEXCEPT override {
            return detail::LcdSpiPeripheralEsp::instance().allocateBuffer(s);
        }
        void freeBuffer(u16 *b) FL_NOEXCEPT override {
            detail::LcdSpiPeripheralEsp::instance().freeBuffer(b);
        }
        bool transmit(const u16 *b, size_t s) FL_NOEXCEPT override {
            return detail::LcdSpiPeripheralEsp::instance().transmit(b, s);
        }
        bool waitTransmitDone(u32 t) FL_NOEXCEPT override {
            return detail::LcdSpiPeripheralEsp::instance().waitTransmitDone(t);
        }
        bool isBusy() const FL_NOEXCEPT override {
            return detail::LcdSpiPeripheralEsp::instance().isBusy();
        }
        bool registerTransmitCallback(void *cb, void *ctx) FL_NOEXCEPT override {
            return detail::LcdSpiPeripheralEsp::instance()
                .registerTransmitCallback(cb, ctx);
        }
        const detail::LcdSpiConfig &getConfig() const FL_NOEXCEPT override {
            return detail::LcdSpiPeripheralEsp::instance().getConfig();
        }
        u64 getMicroseconds() FL_NOEXCEPT override {
            return detail::LcdSpiPeripheralEsp::instance().getMicroseconds();
        }
        void delay(u32 ms) FL_NOEXCEPT override {
            detail::LcdSpiPeripheralEsp::instance().delay(ms);
        }
    };
    auto wrapper = fl::make_shared<EspWrapper>();
    return fl::make_shared<ChannelDriverLcdSpi>(wrapper);
#else
    return nullptr;
#endif
}

} // namespace fl

#undef FL_LCD_SPI_COMPILE

#endif // FL_LCD_SPI_COMPILE
