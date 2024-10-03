
// g++ --std=c++11 test.cpp

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <stdint.h>

#include "doctest.h"
#include "fx/cylon.hpp"
#include "fx/pride2015.hpp"
#include "fx/noisewave.hpp"
#include "fx/_fx2d.h"
#include "fx/_fxstrip.h"
#include "fx/_xmap.h"
#include <chrono>

#define COMPILE_ANIMARTRIX 0

#if COMPILE_ANIMARTRIX
#include "Arduino.h"
#include "fx/animartrix.hpp"
#endif

// To satisfy the linker, we must also define uint16_t XY( uint8_t, uint8_t);
// This should go away someday and only use functions supplied by the user.
uint16_t XY( uint8_t, uint8_t) { return 0; }

// Implement millis() function using std::chrono
uint32_t millis() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

TEST_CASE("Compile Test") {
    // Suceessful compilation test
}
