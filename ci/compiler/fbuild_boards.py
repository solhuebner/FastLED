"""Boards that use fbuild instead of PlatformIO for compilation."""

# These boards use fbuild (Rust-based build system) instead of direct PlatformIO CLI.
# fbuild provides faster builds via daemon-based caching and toolchain management.
#
# Supported platforms (fbuild orchestrator must exist):
#   - atmelavr  -> AvrOrchestrator  (avr-gcc with -mmcu=)
#   - espressif32 (pioarduino) -> Esp32Orchestrator (metadata-driven toolchain)
#   - teensy    -> TeensyOrchestrator (arm-none-eabi-gcc, Cortex-M7 only)
#
# NOT supported by fbuild (must stay on PlatformIO):
#   - atmelmegaavr boards (ATtiny1604, ATtiny1616, nano_every) -- platform not recognized
#   - esp32s2, esp32h2
#   - Teensy 3.x/LC (teensy30..36, teensylc)
#   - Specialized ESP32 variants (qemu, idf33, idf44, i2s)
#   - uno_r4_wifi -- uses renesas-ra platform, no fbuild orchestrator
#   - ESP32 boards (Windows)
#   - teensy40, teensy41
FBUILD_BOARDS: frozenset[str] = frozenset(
    {
        # AVR (atmelavr)
        "uno",
        "attiny85",
        "attiny88",
        "attiny4313",
        "leonardo",
        # ESP32 (pioarduino espressif32)
        # "esp32dev",
        # "esp32s3",
        # "esp32c3",
        # "esp32c6",
        # "esp32c2",
        # "esp32c5",
        # "esp32p4",
        # "esp32s2",
        # "esp32h2",
        # "upesy_wroom",
        # "seeed_xiao_esp32s3",
    }
)
