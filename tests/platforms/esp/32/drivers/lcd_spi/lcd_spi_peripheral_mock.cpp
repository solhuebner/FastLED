/// @file lcd_spi_peripheral_mock.cpp
/// @brief Unit tests for LCD_CAM SPI mock peripheral

#ifdef FASTLED_STUB_IMPL

#include "test.h"
#include "platforms/esp/32/drivers/lcd_spi/lcd_spi_peripheral_mock.h"
#include "fl/stl/vector.h"

FL_TEST_FILE(FL_FILEPATH) {

using namespace fl;
using namespace fl::detail;

namespace {

void resetMockState() {
    auto &mock = LcdSpiPeripheralMock::instance();
    mock.reset();
}

} // anonymous namespace

FL_TEST_CASE("LcdSpiPeripheralMock - basic initialization") {
    resetMockState();
    auto &mock = LcdSpiPeripheralMock::instance();

    FL_CHECK_FALSE(mock.isInitialized());

    LcdSpiConfig config;
    config.num_lanes = 4;
    config.clock_gpio = 18;
    config.clock_hz = 6000000;
    config.max_transfer_bytes = 4096;
    config.data_gpios[0] = 1;
    config.data_gpios[1] = 2;
    config.data_gpios[2] = 3;
    config.data_gpios[3] = 4;

    FL_CHECK(mock.initialize(config));
    FL_CHECK(mock.isInitialized());
    FL_CHECK(mock.isEnabled());

    const auto &stored = mock.getConfig();
    FL_CHECK(stored.clock_hz == 6000000);
    FL_CHECK(stored.num_lanes == 4);
    FL_CHECK(stored.clock_gpio == 18);
}

FL_TEST_CASE("LcdSpiPeripheralMock - invalid configuration") {
    resetMockState();
    auto &mock = LcdSpiPeripheralMock::instance();

    LcdSpiConfig config;
    config.clock_hz = 6000000;
    config.num_lanes = 0;
    FL_CHECK_FALSE(mock.initialize(config));

    config.num_lanes = 17;
    FL_CHECK_FALSE(mock.initialize(config));
}

FL_TEST_CASE("LcdSpiPeripheralMock - buffer allocation") {
    resetMockState();
    auto &mock = LcdSpiPeripheralMock::instance();

    LcdSpiConfig config(1, 18, 6000000, 4096);
    FL_REQUIRE(mock.initialize(config));

    u16 *buffer = mock.allocateBuffer(1024);
    FL_REQUIRE(buffer != nullptr);

    for (size_t i = 0; i < 512; i++) {
        buffer[i] = static_cast<u16>(i);
    }
    for (size_t i = 0; i < 512; i++) {
        FL_CHECK(buffer[i] == static_cast<u16>(i));
    }

    mock.freeBuffer(buffer);
}

FL_TEST_CASE("LcdSpiPeripheralMock - basic transmit") {
    resetMockState();
    auto &mock = LcdSpiPeripheralMock::instance();

    LcdSpiConfig config(4, 18, 6000000, 4096);
    FL_REQUIRE(mock.initialize(config));

    u16 *buffer = mock.allocateBuffer(256);
    FL_REQUIRE(buffer != nullptr);

    for (size_t i = 0; i < 128; i++) {
        buffer[i] = 0xAAAA;
    }

    FL_CHECK(mock.transmit(buffer, 256));
    FL_CHECK(mock.isBusy()); // Async: still busy after transmit

    mock.simulateTransmitComplete();
    FL_CHECK_FALSE(mock.isBusy());

    const auto &history = mock.getTransmitHistory();
    FL_CHECK(history.size() == 1);
    FL_CHECK(history[0].size_bytes == 256);
    FL_CHECK(mock.getTransmitCount() == 1);

    mock.freeBuffer(buffer);
}

FL_TEST_CASE("LcdSpiPeripheralMock - transmit data capture") {
    resetMockState();
    auto &mock = LcdSpiPeripheralMock::instance();

    LcdSpiConfig config(2, 18, 6000000, 1024);
    FL_REQUIRE(mock.initialize(config));

    u16 *buffer = mock.allocateBuffer(64);
    FL_REQUIRE(buffer != nullptr);

    for (size_t i = 0; i < 32; i++) {
        buffer[i] = static_cast<u16>(0x1234 + i);
    }

    FL_REQUIRE(mock.transmit(buffer, 64));
    mock.simulateTransmitComplete();

    fl::span<const u16> last_data = mock.getLastTransmitData();
    FL_REQUIRE(last_data.size() == 32);

    for (size_t i = 0; i < last_data.size(); i++) {
        FL_CHECK(last_data[i] == static_cast<u16>(0x1234 + i));
    }

    mock.freeBuffer(buffer);
}

FL_TEST_CASE("LcdSpiPeripheralMock - transmit failure injection") {
    resetMockState();
    auto &mock = LcdSpiPeripheralMock::instance();

    LcdSpiConfig config(1, 18, 6000000, 1024);
    FL_REQUIRE(mock.initialize(config));

    u16 *buffer = mock.allocateBuffer(256);
    FL_REQUIRE(buffer != nullptr);

    mock.setTransmitFailure(true);
    FL_CHECK_FALSE(mock.transmit(buffer, 256));

    mock.setTransmitFailure(false);
    FL_CHECK(mock.transmit(buffer, 256));

    mock.freeBuffer(buffer);
}

FL_TEST_CASE("LcdSpiPeripheralMock - transmit without initialization") {
    resetMockState();
    auto &mock = LcdSpiPeripheralMock::instance();

    u16 dummy[16] = {0};
    FL_CHECK_FALSE(mock.transmit(dummy, sizeof(dummy)));
}

FL_TEST_CASE("LcdSpiPeripheralMock - reset clears all state") {
    resetMockState();
    auto &mock = LcdSpiPeripheralMock::instance();

    LcdSpiConfig config(1, 18, 6000000, 1024);
    FL_REQUIRE(mock.initialize(config));

    u16 *buffer = mock.allocateBuffer(256);
    FL_REQUIRE(buffer != nullptr);
    FL_REQUIRE(mock.transmit(buffer, 256));
    mock.simulateTransmitComplete();
    mock.freeBuffer(buffer);

    mock.reset();

    FL_CHECK_FALSE(mock.isInitialized());
    FL_CHECK_FALSE(mock.isEnabled());
    FL_CHECK_FALSE(mock.isBusy());
    FL_CHECK(mock.getTransmitCount() == 0);
    FL_CHECK(mock.getTransmitHistory().size() == 0);
}

FL_TEST_CASE("LcdSpiPeripheralMock - deinitialize") {
    resetMockState();
    auto &mock = LcdSpiPeripheralMock::instance();

    LcdSpiConfig config(1, 18, 6000000, 1024);
    FL_REQUIRE(mock.initialize(config));
    FL_CHECK(mock.isInitialized());

    mock.deinitialize();
    FL_CHECK_FALSE(mock.isInitialized());
}

#endif // FASTLED_STUB_IMPL

} // FL_TEST_FILE
