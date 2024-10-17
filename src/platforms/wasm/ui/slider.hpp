#include "platforms/wasm/js.h"
#include "ui_manager.h"
#include <sstream>
#include <string.h>
#include "json.h"

FASTLED_NAMESPACE_BEGIN



jsSlider::jsSlider(const char* name, float value, float min, float max, float step)
    : mMin(min), mMax(max), mValue(value), mStep(step) {
    auto updateFunc = [this](const char* jsonStr) { this->updateInternal(jsonStr); };
    auto toJsonFunc = [this](ArduinoJson::JsonObject& json) { this->toJson(json); };
    mInternal = std::make_shared<jsUiInternal>(name, std::move(updateFunc), std::move(toJsonFunc));
    jsUiManager::addComponent(mInternal);
}

jsSlider::~jsSlider() {
    jsUiManager::removeComponent(mInternal);
}

const char* jsSlider::name() const {
    return mInternal->name();
}

void jsSlider::toJson(ArduinoJson::JsonObject& json) const {
    json["name"] = name();
    json["type"] = "slider";
    json["id"] = mInternal->id();
    json["min"] = mMin;
    json["max"] = mMax;
    json["value"] = mValue;
    json["step"] = mStep;
}

float jsSlider::value() const { 
    return mValue; 
}

void jsSlider::updateInternal(const char* jsonStr) {
    // We expect jsonStr to actually be a value string, so simply parse it.
    float value = std::stof(jsonStr);
    setValue(value);
}

void jsSlider::setValue(float value) {
    mValue = std::max(mMin, std::min(mMax, value));
    if (mValue != value) {
        // The value was outside the range so print out a warning that we
        // clamped.
        const char* name = mInternal->name();
        int id = mInternal->id();
        printf(
            "Warning: Slider %s with id %d value %f was clamped to range [%f, %f] -> %f\n",
            name, id,
            value, mMin, mMax, mValue);
    }
}


jsSlider::operator float() const { 
    return mValue; 
}

jsSlider::operator uint8_t() const {
    return static_cast<uint8_t>(mValue);
}

jsSlider::operator uint16_t() const {
    return static_cast<uint8_t>(mValue);
}

jsSlider::operator int() const {
    return static_cast<int>(mValue);
}

FASTLED_NAMESPACE_END