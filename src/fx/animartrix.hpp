/*
  ___        _            ___  ______ _____    _
 / _ \      (_)          / _ \ | ___ \_   _|  (_)
/ /_\ \_ __  _ _ __ ___ / /_\ \| |_/ / | |_ __ ___  __
|  _  | '_ \| | '_ ` _ \|  _  ||    /  | | '__| \ \/ /
| | | | | | | | | | | | | | | || |\ \  | | |  | |>  <
\_| |_/_| |_|_|_| |_| |_\_| |_/\_| \_| \_/_|  |_/_/\_\

by Stefan Petrick 2023.

High quality LED animations for your next project.

This is a Shader and 5D Coordinate Mapper made for realtime
rendering of generative animations & artistic dynamic visuals.

This is also a modular animation synthesizer with waveform
generators, oscillators, filters, modulators, noise generators,
compressors... and much more.

VO.42 beta version

This code is licenced under a Creative Commons Attribution
License CC BY-NC 3.0


*/

#warning "\
ANIMartRIX: free for non-commercial use and licensed under the Creative Commons Attribution License CC BY-NC-SA 4.0, . \
For commercial use, please contact Stefan Petrick. Github: https://github.com/StefanPetrick/animartrix". \
Modified by github.com/netmindz for class portability. \
Modified by Zach Vorhies for FastLED fx compatibility."

#include "animartrix.h"
#include <FastLED.h>

#include <iostream>

enum Animation {
    RGB_BLOBS5 = 0,
    RGB_BLOBS4,
    RGB_BLOBS3,
    RGB_BLOBS2,
    RGB_BLOBS,
    POLAR_WAVES,
    SLOW_FADE,
    ZOOM2,
    ZOOM,
    HOT_BLOB,
    SPIRALUS2,
    SPIRALUS,
    YVES,
    SCALEDEMO1,
    LAVA1,
    CALEIDO3,
    CALEIDO2,
    CALEIDO1,
    DISTANCE_EXPERIMENT,
    CENTER_FIELD,
    WAVES,
    CHASING_SPIRALS,
    ROTATING_BLOB,
    RINGS,
    NUM_ANIMATIONS
};


class FastLEDANIMartRIX;
class AnimatrixData {
  public:
    int x = 0;
    int y = 0;
    bool serpentine = true;
    FastLEDANIMartRIX* obj = nullptr;
    bool destroy = false;
    CRGB *leds = nullptr;
    Animation current_animation = RGB_BLOBS5;

    AnimatrixData(int x, int y, CRGB *leds, Animation first_animation, bool serpentine) {
        this->x = x;
        this->y = y;
        this->leds = leds;
        this->serpentine = serpentine;
        this->current_animation = first_animation;
    }
};

void AnimatrixDataLoop(AnimatrixData &self);





class FastLEDANIMartRIX : public ANIMartRIX {
    AnimatrixData *data = nullptr;

  public:
    FastLEDANIMartRIX(AnimatrixData *_data) {
        this->data = _data;
        this->init(data->x, data->y, serpentine);
    }
    void setPixelColor(int x, int y, rgb pixel) {
        data->leds[xy(x, y)] = CRGB(pixel.red, pixel.green, pixel.blue);
    }
    void setPixelColor(int index, rgb pixel) {
        data->leds[index] = CRGB(pixel.red, pixel.green, pixel.blue);
    }

    void loop() {
        switch (data->current_animation) {
        case RGB_BLOBS5:
            RGB_Blobs5();
            break;
        case RGB_BLOBS4:
            RGB_Blobs4();
            break;
        case RGB_BLOBS3:
            RGB_Blobs3();
            break;
        case RGB_BLOBS2:
            RGB_Blobs2();
            break;
        case RGB_BLOBS:
            RGB_Blobs();
            break;
        case POLAR_WAVES:
            Polar_Waves();
            break;
        case SLOW_FADE:
            Slow_Fade();
            break;
        case ZOOM2:
            Zoom2();
            break;
        case ZOOM:
            Zoom();
            break;
        case HOT_BLOB:
            Hot_Blob();
            break;
        case SPIRALUS2:
            Spiralus2();
            break;
        case SPIRALUS:
            Spiralus();
            break;
        case YVES:
            Yves();
            break;
        case SCALEDEMO1:
            Scaledemo1();
            break;
        case LAVA1:
            Lava1();
            break;
        case CALEIDO3:
            Caleido3();
            break;
        case CALEIDO2:
            Caleido2();
            break;
        case CALEIDO1:
            Caleido1();
            break;
        case DISTANCE_EXPERIMENT:
            Distance_Experiment();
            break;
        case CENTER_FIELD:
            Center_Field();
            break;
        case WAVES:
            Waves();
            break;
        case CHASING_SPIRALS:
            Chasing_Spirals();
            break;
        case ROTATING_BLOB:
            Rotating_Blob();
            break;
        case RINGS:
            Rings();
            break;
        }
    }
};



void AnimatrixDataLoop(AnimatrixData &self) {
    if (self.obj == nullptr) {
        self.obj = new FastLEDANIMartRIX(&self);
    }
    if (self.destroy) {
        delete self.obj;
        self.obj = nullptr;
        self.destroy = false;
        return;
    }
    std::cout << "looping" << std::endl;
    self.obj->loop();
}