
#pragma once

// This is a drawing/graphics related class.
//
// XYPath represents a parameterized (x,y) path. The input will always be
// an alpha value between 0->1 (float) or 0->0xffff (uint16_t).
// A look up table can be used to optimize path calculations when steps > 0.
//
// We provide common paths discovered throughout human history, for use in
// your animations.

#include "fl/function.h"
#include "fl/ptr.h"
#include "fl/tile2x2.h"
#include "fl/transform.h"
#include "fl/xypath_impls.h"
#include "fl/pair.h"

namespace fl {

class XYRasterU8Sparse;
template <typename T> class function;

// Smart pointers for the XYPath family.
FASTLED_SMART_PTR(XYPath);
FASTLED_SMART_PTR(XYPathRenderer);
FASTLED_SMART_PTR(XYPathGenerator);
FASTLED_SMART_PTR(XYPathFunction);

namespace xypath_detail {
fl::Str unique_missing_name(const fl::Str &prefix = "XYCustomPath: ");
} // namespace xypath_detail

class XYPath : public Referent {
  public:
    /////////////////////////////////////////////
    // Begin pre-baked paths.
    // Point
    static XYPathPtr NewPointPath(float x, float y);
    // Lines and curves
    static XYPathPtr NewLinePath(float x0, float y0, float x1, float y1);
    static XYPathPtr
    NewLinePath(const Ptr<LinePathParams> &params = NewPtr<LinePathParams>());
    // Cutmull allows for a path to be defined by a set of points. The path will
    // be a smooth curve through the points.
    static XYPathPtr NewCatmullRomPath(
        uint16_t width = 0, uint16_t height = 0,
        const Ptr<CatmullRomParams> &params = NewPtr<CatmullRomParams>());

    // Custom path using just a function.
    static XYPathPtr
    NewCustomPath(const fl::function<point_xy_float(float)> &path,
                  const rect_xy<int> &drawbounds = rect_xy<int>(),
                  const TransformFloat &transform = TransformFloat(),
                  const Str &name = xypath_detail::unique_missing_name());

    static XYPathPtr NewCirclePath();
    static XYPathPtr NewCirclePath(uint16_t width, uint16_t height);
    static XYPathPtr NewHeartPath();
    static XYPathPtr NewHeartPath(uint16_t width, uint16_t height);
    static XYPathPtr NewArchimedeanSpiralPath(uint16_t width, uint16_t height);
    static XYPathPtr NewArchimedeanSpiralPath();

    static XYPathPtr
    NewRosePath(uint16_t width = 0, uint16_t height = 0,
                const Ptr<RosePathParams> &params = NewPtr<RosePathParams>());

    static XYPathPtr NewPhyllotaxisPath(
        uint16_t width = 0, uint16_t height = 0,
        const Ptr<PhyllotaxisParams> &args = NewPtr<PhyllotaxisParams>());

    static XYPathPtr NewGielisCurvePath(
        uint16_t width = 0, uint16_t height = 0,
        const Ptr<GielisCurveParams> &params = NewPtr<GielisCurveParams>());
    // END pre-baked paths.
    /////////////////////////////////////////////////////////////

    // Create a new Catmull-Rom spline path with custom parameters

    XYPath(XYPathGeneratorPtr path,
           TransformFloat transform = TransformFloat());

    // Future work: we don't actually want just the point, but also
    // it's intensity at that value. Otherwise a seperate class has to
    // made to also control the intensity and that sucks.
    using xy_brightness = fl::pair<point_xy_float, uint8_t>;

    virtual ~XYPath();
    point_xy_float at(float alpha);
    Tile2x2_u8 at_subpixel(float alpha);
    void rasterize(float from, float to, int steps, XYRasterU8Sparse &raster,
                   fl::function<uint8_t(float)> *optional_alpha_gen = nullptr);

    void draw(const CRGB &color, const XYMap &xyMap, CRGB *leds) ;
    void setScale(float scale);
    Str name() const;
    // Overloaded to allow transform to be passed in.
    point_xy_float at(float alpha, const TransformFloat &tx);
    xy_brightness at_brightness(float alpha) {
        point_xy_float p = at(alpha);
        return xy_brightness(p, 0xff);  // Full brightness for now.
    }
    // Needed for drawing to the screen. When this called the rendering will
    // be centered on the width and height such that 0,0 -> maps to .5,.5,
    // which is convenient for drawing since each float pixel can be truncated
    // to an integer type.
    void setDrawBounds(uint16_t width, uint16_t height);
    TransformFloat &transform();

    void setTransform(const TransformFloat &transform);

  private:
    XYPathGeneratorPtr mPath;
    XYPathRendererPtr mPathRenderer;
};

class XYPathFunction : public XYPathGenerator {
  public:
    XYPathFunction(fl::function<point_xy_float(float)> f) : mFunction(f) {}
    point_xy_float compute(float alpha) override { return mFunction(alpha); }
    const Str name() const override { return mName; }
    void setName(const Str &name) { mName = name; }

    fl::rect_xy<int> drawBounds() const { return mDrawBounds; }
    void setDrawBounds(const fl::rect_xy<int> &bounds) { mDrawBounds = bounds; }

    bool hasDrawBounds(fl::rect_xy<int> *bounds) override {
        if (bounds) {
            *bounds = mDrawBounds;
        }
        return true;
    }

  private:
    fl::function<point_xy_float(float)> mFunction;
    fl::Str mName = "XYPathFunction Unnamed";
    fl::rect_xy<int> mDrawBounds;
};

} // namespace fl
