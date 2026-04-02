#pragma once

// Three-tier memory classification:
//   Low:  SKETCH_HAS_LARGE_MEMORY=0, SKETCH_HAS_HUGE_MEMORY=0
//         (AVR, ESP8266, Teensy LC/3.0/3.1/3.2, STM32F1, Renesas UNO)
//   High: SKETCH_HAS_LARGE_MEMORY=1, SKETCH_HAS_HUGE_MEMORY=0
//         (Apollo3, nRF52, SAMD21, generic ARM)
//   Huge: SKETCH_HAS_LARGE_MEMORY=1, SKETCH_HAS_HUGE_MEMORY=1
//         (ESP32, Teensy 3.5/3.6/4.x, RP2040/RP2350, SAMD51, STM32F4+/H7, native/WASM)
//
// Invariant: HUGE=1 implies LARGE=1. LARGE=0 implies HUGE=0.

#ifdef SKETCH_HAS_LARGE_MEMORY
// User has manually defined SKETCH_HAS_LARGE_MEMORY via build flags.
// Mark it as overridden so platform compile tests don't fire.
#define SKETCH_HAS_LARGE_MEMORY_OVERRIDDEN 1
#else
#if defined(FL_IS_AVR) \
  || defined(__AVR_ATtiny85__) \
  || defined(__AVR_ATtiny88__) \
  || defined(__AVR_ATmega32U4__) \
  || defined(ARDUINO_attinyxy6) \
  || defined(ARDUINO_attinyxy4) \
  || defined(FL_IS_TEENSY_LC) \
  || defined(FL_IS_TEENSY_30) \
  || defined(FL_IS_TEENSY_31) \
  || defined(FL_IS_TEENSY_32) \
  || defined(FL_IS_STM32_F1) \
  || defined(FL_IS_ESP8266) \
  || defined(ARDUINO_ARCH_RENESAS_UNO) \
  || defined(ARDUINO_BLUEPILL_F103C8)
#define SKETCH_HAS_LARGE_MEMORY 0
#else
#define SKETCH_HAS_LARGE_MEMORY 1
#endif
#endif

#ifdef SKETCH_HAS_HUGE_MEMORY
// User has manually defined SKETCH_HAS_HUGE_MEMORY via build flags.
#define SKETCH_HAS_HUGE_MEMORY_OVERRIDDEN 1
#else
// Huge memory platforms: >= 256KB RAM, >= 512KB flash.
// ESP32 (all), Teensy 4.x/3.5/3.6, RP2040/RP2350, SAMD51, STM32F4+/H7, native/WASM.
#if defined(FL_IS_ESP32) \
  || defined(FL_IS_TEENSY_35) \
  || defined(FL_IS_TEENSY_36) \
  || defined(FL_IS_TEENSY_4X) \
  || defined(ARDUINO_ARCH_RP2040) \
  || defined(PICO_RP2040) \
  || defined(PICO_RP2350) \
  || defined(__SAMD51__) \
  || defined(STM32F4xx) || defined(STM32H7xx) || defined(ARDUINO_GIGA) \
  || defined(FASTLED_STUB_IMPL) \
  || defined(__EMSCRIPTEN__)
#define SKETCH_HAS_HUGE_MEMORY 1
#else
#define SKETCH_HAS_HUGE_MEMORY 0
#endif
#endif

// Enforce invariant: HUGE=1 requires LARGE=1.
#if SKETCH_HAS_HUGE_MEMORY && !SKETCH_HAS_LARGE_MEMORY
#error "SKETCH_HAS_HUGE_MEMORY=1 requires SKETCH_HAS_LARGE_MEMORY=1"
#endif

// Backward compatibility aliases for external user code.
#ifndef SKETCH_HAS_LOTS_OF_MEMORY
#define SKETCH_HAS_LOTS_OF_MEMORY SKETCH_HAS_LARGE_MEMORY
#endif
#ifndef SKETCH_HAS_VERY_LARGE_MEMORY
#define SKETCH_HAS_VERY_LARGE_MEMORY SKETCH_HAS_HUGE_MEMORY
#endif

#ifndef SKETCH_STRINGIFY
#define SKETCH_STRINGIFY_HELPER(x) #x
#define SKETCH_STRINGIFY(x) SKETCH_STRINGIFY_HELPER(x)
#endif

// SKETCH_HALT and SKETCH_HALT_OK macros have been removed.
// They caused watchdog timer resets on ESP32-C6 due to the infinite while(1) loop
// preventing loop() from returning.
//
// Replacement: Use a static flag to run tests once:
//
// Pattern:
//   static bool tests_run = false;
//
//   void loop() {
//       if (tests_run) {
//           delay(1000);
//           return;
//       }
//       tests_run = true;
//
//       // ... your test code here ...
//       FL_PRINT("Tests complete");
//   }
//
// This allows loop() to return normally, preventing watchdog timer resets.
