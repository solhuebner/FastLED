"""Boards that use fbuild instead of PlatformIO for compilation."""

# These boards use fbuild (Rust-based build system) instead of direct PlatformIO CLI.
# fbuild provides faster builds via daemon-based caching and toolchain management.
#
# Supported platforms (fbuild orchestrator must exist):
#   - atmelavr  -> AvrOrchestrator  (avr-gcc with -mmcu=)
#   - espressif32 (pioarduino) -> Esp32Orchestrator (metadata-driven toolchain)
#   - teensy    -> TeensyOrchestrator (arm-none-eabi-gcc)
#
# NOT supported by fbuild (must stay on PlatformIO):
#   - atmelmegaavr boards (ATtiny1604, ATtiny1616, nano_every) -- orchestrator not implemented
#   - Teensy boards -- lib/ discovery fails + command-line quoting broken on Windows
#   - atmega8a -- MiniCore framework: core path + board name mapping broken
#   - Specialized ESP32 variants (qemu, idf33, idf44, i2s, rmt_51) -- custom IDF/driver configs
#   - uno_r4_wifi -- uses renesas-ra platform, no fbuild orchestrator
#   - esp8266 -- no orchestrator
#   - STM32, RP2040/RP2350, NRF52, Apollo3, SAM/SAMD, MGM240 -- no orchestrators
FBUILD_BOARDS: frozenset[str] = frozenset(
    {
        # AVR (atmelavr)
        "uno",
        "attiny85",
        "attiny88",
        "attiny4313",
        "leonardo",
        # ESP32 (pioarduino espressif32)
        "esp32dev",
        "esp32s3",
        "esp32c3",
        "esp32c6",
        "esp32c2",
        "esp32c5",
        "esp32p4",
        "esp32s2",
        "esp32h2",
        "upesy_wroom",
        "seeed_xiao_esp32s3",
    }
)
