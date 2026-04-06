/// @file channel_driver_lcd_spi.h
/// @brief LCD_CAM SPI channel driver for true SPI chipsets (APA102, SK9822)
///
/// Uses LCD_CAM I80 bus on ESP32-S3 to drive clocked SPI LED strips.
/// Handles SPI chipsets only (not clockless). Data is transposed for
/// parallel 16-bit output on the I80 bus.

#pragma once

// IWYU pragma: private

#include "fl/stl/compiler_control.h"
#include "fl/channels/data.h"
#include "fl/channels/driver.h"
#include "fl/stl/noexcept.h"
#include "fl/stl/span.h"
#include "fl/stl/shared_ptr.h"
#include "fl/stl/vector.h"
#include "platforms/esp/32/drivers/lcd_spi/ilcd_spi_peripheral.h"

namespace fl {

/// @brief Channel driver for SPI chipsets via LCD_CAM I80 bus
///
/// Implements IChannelDriver for true SPI protocols (APA102, SK9822, HD108).
/// Uses LCD_CAM peripheral with PCLK as SPI clock and D0-D15 as parallel
/// data lanes. Data bytes are transposed into 16-bit words for I80 output.
///
/// Currently blocking: show() waits for transmission to complete.
class ChannelDriverLcdSpi : public IChannelDriver {
  public:
    explicit ChannelDriverLcdSpi(
        fl::shared_ptr<detail::ILcdSpiPeripheral> peripheral) FL_NOEXCEPT;
    ~ChannelDriverLcdSpi() override;

    bool canHandle(const ChannelDataPtr &data) const FL_NOEXCEPT override;
    void enqueue(ChannelDataPtr channelData) FL_NOEXCEPT override;
    void show() FL_NOEXCEPT override;
    DriverState poll() FL_NOEXCEPT override;

    fl::string getName() const FL_NOEXCEPT override {
        return fl::string::from_literal("LCD_SPI");
    }

    Capabilities getCapabilities() const FL_NOEXCEPT override {
        return Capabilities(false, true); // SPI only
    }

  private:
    bool beginTransmission(
        fl::span<const ChannelDataPtr> channels) FL_NOEXCEPT;

    /// @brief Transpose byte data into 16-bit words for I80 bus output
    /// Each bit position in the output word corresponds to one data lane.
    void transposeToWords(fl::span<const ChannelDataPtr> channels,
                          u16 *output, size_t maxBytes) FL_NOEXCEPT;

    fl::shared_ptr<detail::ILcdSpiPeripheral> mPeripheral;
    bool mInitialized;
    fl::vector<ChannelDataPtr> mEnqueuedChannels;
    fl::vector<ChannelDataPtr> mTransmittingChannels;

    u16 *mBuffer;
    size_t mBufferSize;
    volatile bool mBusy;
};

/// @brief Factory function to create LCD_SPI driver with real hardware
fl::shared_ptr<IChannelDriver> createLcdSpiEngine() FL_NOEXCEPT;

} // namespace fl
