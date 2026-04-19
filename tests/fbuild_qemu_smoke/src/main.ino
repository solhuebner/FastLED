// Minimal FastLED sketch for fbuild native-QEMU smoke test.
// Must print FBUILD-QEMU-TEST-OK once early init completes so the QEMU
// runner's halt-on-success matcher can exit the emulator cleanly.

#include <Arduino.h>
#include <FastLED.h>

#define NUM_LEDS 1
#define PIN_DATA 5

CRGB leds[NUM_LEDS];

void setup() {
    Serial.begin(115200);
    Serial.println("FBUILD-QEMU-TEST-BOOT");

    FastLED.addLeds<WS2812, PIN_DATA, GRB>(leds, NUM_LEDS);
    leds[0] = CRGB::Red;
    FastLED.show();

    Serial.println("FBUILD-QEMU-TEST-OK");
}

void loop() {
    delay(1000);
}
