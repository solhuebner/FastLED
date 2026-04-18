#include "fl/ui.h"
#include "fl/audio/audio_manager.h"
#include "fl/stl/stdint.h"
#include "fl/stl/compiler_control.h"
#include "fl/stl/noexcept.h"

FL_DISABLE_WARNING_PUSH
FL_DISABLE_WARNING(float-equal)

namespace fl {

// UIElement constructor
UIElement::UIElement() {}

// UISlider constructor
UISlider::UISlider(const char *name, float value, float min, float max, float step) FL_NOEXCEPT
    : mImpl(name, value, min, max, step), mListener(this) {
    mListener.addToEngineEventsOnce();
}

// UIButton constructor
UIButton::UIButton(const char *name) FL_NOEXCEPT : mImpl(name), mListener(this) {
    mListener.addToEngineEventsOnce();
}

UIButton::~UIButton() FL_NOEXCEPT {}

// UICheckbox constructor
UICheckbox::UICheckbox(const char *name, bool value) FL_NOEXCEPT
    : mImpl(name, value), mLastFrameValue(false), mLastFrameValueValid(false), mListener(this) {
    mListener.addToEngineEventsOnce();
}

UICheckbox::~UICheckbox() FL_NOEXCEPT {}

// UINumberField constructor
UINumberField::UINumberField(const char *name, double value, double min, double max) FL_NOEXCEPT
    : mImpl(name, value, min, max), mListener(this), mLastFrameValue(0), mLastFrameValueValid(false) {
    mListener.addToEngineEventsOnce();
}

UINumberField::~UINumberField() FL_NOEXCEPT {}

// UITitle constructors
#if FASTLED_USE_JSON_UI
UITitle::UITitle(const char *name) FL_NOEXCEPT : mImpl(fl::string(name), fl::string(name)) {}
#else
UITitle::UITitle(const char *name) : mImpl(name) {}
#endif

UITitle::~UITitle() FL_NOEXCEPT {}

// UIDescription constructor
UIDescription::UIDescription(const char *name) FL_NOEXCEPT : mImpl(name) {}
UIDescription::~UIDescription() FL_NOEXCEPT {}

// UIHelp constructor
UIHelp::UIHelp(const char *markdownContent) FL_NOEXCEPT : mImpl(markdownContent) {}
UIHelp::~UIHelp() FL_NOEXCEPT {}

// UIAudio constructors
UIAudio::UIAudio(const fl::string& name) FL_NOEXCEPT : mImpl(name) {}
UIAudio::UIAudio(const fl::string& name, const fl::url& url) FL_NOEXCEPT : mImpl(name, url) {}

// Asset-handle overload (issue #2284): resolves the asset via the
// v1 asset pipeline (registry + host/stub filesystem fallback) and
// forwards to the url-based ctor. If resolution fails, we forward
// the unresolved url so the consumer still sees a url field — the
// existing JsonAudioImpl code emits an empty string for an invalid
// url, matching the name-only behavior.
UIAudio::UIAudio(const fl::string& name, const fl::asset_ref& asset) FL_NOEXCEPT
    : mImpl(name, fl::resolve_asset(asset)) {}

UIAudio::UIAudio(const fl::string& name, const fl::audio::Config& config) FL_NOEXCEPT : mImpl(name, config), mConfig(config) {}
UIAudio::~UIAudio() FL_NOEXCEPT {}

fl::shared_ptr<audio::Processor> UIAudio::processor() FL_NOEXCEPT {
    if (!mProcessor) {
        mProcessor = audio::AudioManager::instance().add(*this);
    }
    return mProcessor;
}

// UIDropdown constructors
UIDropdown::UIDropdown(const char *name, fl::span<fl::string> options) FL_NOEXCEPT
    : mImpl(fl::string(name), options), mListener(this) {
    mListener.addToEngineEventsOnce();
}

UIDropdown::UIDropdown(const char *name, fl::initializer_list<fl::string> options) FL_NOEXCEPT
    : mImpl(name, options), mListener(this) {
    mListener.addToEngineEventsOnce();
}

UIDropdown::~UIDropdown() FL_NOEXCEPT {}

// UIGroup constructors
UIGroup::UIGroup(const fl::string& groupName) FL_NOEXCEPT : mImpl(groupName.c_str()) {}
UIGroup::~UIGroup() FL_NOEXCEPT {}

void UISlider::setValue(float value) FL_NOEXCEPT {
    float oldValue = mImpl.value();
    if (value != oldValue) {
        mImpl.setValue(value);
        // Update the last frame value to keep state consistent
        mLastFrameValue = value;
        mLastFrameValueValid = true;
        // Invoke callbacks to notify listeners (including JavaScript components)
        mCallbacks.invoke(*this);
    }
}

void UISlider::Listener::onBeginFrame() FL_NOEXCEPT {
    UISlider &owner = *mOwner;
    if (!owner.mLastFrameValueValid) {
        owner.mLastFrameValue = owner.value();
        owner.mLastFrameValueValid = true;
        return;
    }
    float value = owner.value();
    if (value != owner.mLastFrameValue) {
        owner.mCallbacks.invoke(*mOwner);
        owner.mLastFrameValue = value;
    }
}

void UIButton::Listener::onBeginFrame() FL_NOEXCEPT {
    bool clicked_this_frame = mOwner->clicked();
    bool pressed_this_frame = mOwner->isPressed();

    // Check the real button if one is attached (via IButtonInput interface)
    if (mOwner->mButtonInput) {
        if (mOwner->mButtonInput->isPressed()) {
            clicked_this_frame = true;
            pressed_this_frame = true;
        }
    }

    // Detect press event (was not pressed, now is pressed)
    if (pressed_this_frame && !mPressedLastFrame) {
        mOwner->mPressCallbacks.invoke();
    }

    // Detect release event (was pressed, now is not pressed)
    if (!pressed_this_frame && mPressedLastFrame) {
        mOwner->mReleaseCallbacks.invoke();
    }

    mPressedLastFrame = pressed_this_frame;

    const bool clicked_changed = (clicked_this_frame != mClickedLastFrame);
    mClickedLastFrame = clicked_this_frame;
    if (clicked_changed) {
        // FL_WARN("Button: " << mOwner->name() << " clicked: " <<
        // mOwner->clicked());
        mOwner->mCallbacks.invoke(*mOwner);
    }
    // mOwner->mCallbacks.invoke(*mOwner);
}

void UICheckbox::Listener::onBeginFrame() FL_NOEXCEPT {
    UICheckbox &owner = *mOwner;
    if (!owner.mLastFrameValueValid) {
        owner.mLastFrameValue = owner.value();
        owner.mLastFrameValueValid = true;
        return;
    }
    bool value = owner.value();
    if (value != owner.mLastFrameValue) {
        owner.mCallbacks.invoke(owner);
        owner.mLastFrameValue = value;
    }
}

void UINumberField::Listener::onBeginFrame() FL_NOEXCEPT {
    UINumberField &owner = *mOwner;
    if (!owner.mLastFrameValueValid) {
        owner.mLastFrameValue = owner.value();
        owner.mLastFrameValueValid = true;
        return;
    }
    double value = owner.value();
    if (value != owner.mLastFrameValue) {
        owner.mCallbacks.invoke(owner);
        owner.mLastFrameValue = value;
    }
}

void UIDropdown::Listener::onBeginFrame() FL_NOEXCEPT {
    UIDropdown &owner = *mOwner;
    
    // Check the next button if one is attached (via IButtonInput interface)
    bool shouldAdvance = false;
    if (owner.mNextButton) {
        if (owner.mNextButton->clicked()) {
            shouldAdvance = true;
        }
    }
    
    // If the next button was clicked, advance to the next option
    if (shouldAdvance) {
        owner.nextOption();
        // The option change will be detected below and callbacks will be invoked
    }
    
    if (!owner.mLastFrameValueValid) {
        owner.mLastFrameValue = owner.as_int();
        owner.mLastFrameValueValid = true;
        return;
    }
    int value = owner.as_int();
    if (value != owner.mLastFrameValue) {
        owner.mCallbacks.invoke(owner);
        owner.mLastFrameValue = value;
    }
}

} // end namespace fl

FL_DISABLE_WARNING_POP
