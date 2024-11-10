#pragma once

#include <stdint.h>

#include "namespace.h"
#include "ref.h"
#include "fx/fx2d.h"


FASTLED_NAMESPACE_BEGIN

// Forward declare dependencies.
FASTLED_SMART_REF(VideoImpl);
FASTLED_SMART_REF(ByteStream);
FASTLED_SMART_REF(FileHandle);
FASTLED_SMART_REF(VideoFx);
FASTLED_SMART_REF(Frame);
struct CRGB;

class Video {
public:
    // frameHistoryCount is the number of frames to keep in the buffer after draw. This
    // allows for time based effects like syncing video speed to audio triggers.
    Video();  // Please use FileSytem to construct a Video.
    ~Video();
    Video(const Video&);
    Video& operator=(const Video&);
    // Api
    void begin(FileHandleRef h, size_t pixelsPerFrame, float fps = 30.0f, size_t frameHistoryCount = 0);
    void beginStream(ByteStreamRef s, size_t pixelsPerFrame, float fps = 30.0f, size_t frameHistoryCount = 0);
    bool draw(uint32_t now, CRGB* leds, uint8_t* alpha = nullptr);
    bool draw(uint32_t now, Frame* frame);
    void end();
    bool finished();
    bool rewind();

    // make compatible with if statements
    operator bool() const { return mImpl.get(); }
private:
    bool mFinished = false;
    VideoImplRef mImpl;
};


class VideoFx : public FxGrid {
  public:
    VideoFx(Video video, XYMap xymap);
    void draw(DrawContext context) override;
    const char *fxName(int) const override;

  private:
    Video mVideo;
    FrameRef mFrame;
};

FASTLED_NAMESPACE_END
