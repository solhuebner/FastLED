#pragma once

/// @file fl/gfx/canvas.h
/// @brief Canvas types for gfx primitives (implementation)

#include "fl/math/alpha.h"
#include "fl/gfx/draw_mode.h"
#include "fl/stl/int.h"
#include "fl/stl/span.h"
#include "fl/stl/variant.h"
#include "fl/stl/shared_ptr.h"
#include "fl/stl/noexcept.h"

namespace fl {
namespace gfx {

/// @brief Line cap styles for stroke operations
enum class LineCap { FLAT, ROUND, SQUARE };

// Forward declaration
template<typename RGB_T> struct Canvas;

// Free function forward declarations
template<int hRadius, int vRadius, typename RGB_T>
void blurGaussian(Canvas<RGB_T>& canvas, fl::alpha8 dimFactor) FL_NOEXCEPT;

template<int hRadius, int vRadius, typename RGB_T>
void blurGaussian(Canvas<RGB_T>& canvas, fl::alpha16 dimFactor) FL_NOEXCEPT;

template<int hRadius, int vRadius, typename RGB_T>
void blurGaussian(Canvas<RGB_T>& canvas) FL_NOEXCEPT;

template<typename PixelT, typename Coord>
void drawLine(Canvas<PixelT>& canvas, const PixelT& color, Coord x0, Coord y0, Coord x1, Coord y1,
              fl::DrawMode mode = fl::DrawMode::DRAW_MODE_BLEND) FL_NOEXCEPT;

template<typename PixelT, typename Coord>
void drawDisc(Canvas<PixelT>& canvas, const PixelT& color, Coord cx, Coord cy, Coord r,
              fl::DrawMode mode = fl::DrawMode::DRAW_MODE_BLEND) FL_NOEXCEPT;

template<typename PixelT, typename Coord>
void drawRing(Canvas<PixelT>& canvas, const PixelT& color, Coord cx, Coord cy, Coord r, Coord thickness,
              fl::DrawMode mode = fl::DrawMode::DRAW_MODE_BLEND) FL_NOEXCEPT;

template<typename PixelT, typename Coord>
void drawStrokeLine(Canvas<PixelT>& canvas, const PixelT& color, Coord x0, Coord y0, Coord x1, Coord y1,
                    Coord thickness, LineCap cap, fl::DrawMode mode = fl::DrawMode::DRAW_MODE_BLEND) FL_NOEXCEPT;

/// @brief Simple rectangular canvas for graphics operations
/// Combines a pixel buffer with dimensions for cache-optimal drawing.
template<typename RGB_T>
struct Canvas {
    fl::variant<
        fl::span<RGB_T>,
        fl::shared_ptr<RGB_T>
    > ownership;

    RGB_T* pixels;
    int width;
    int height;

    Canvas(fl::span<RGB_T> buf, int w, int h) FL_NOEXCEPT
        : ownership(buf), pixels(buf.data()), width(w), height(h) {}
    Canvas(fl::shared_ptr<RGB_T> ptr, int w, int h) FL_NOEXCEPT
        : ownership(ptr), pixels(ptr.get()), width(w), height(h) {}

    int size() const FL_NOEXCEPT { return width * height; }
    RGB_T& at(int x, int y) FL_NOEXCEPT { return pixels[y * width + x]; }
    const RGB_T& at(int x, int y) const FL_NOEXCEPT { return pixels[y * width + x]; }
    bool has(int x, int y) const FL_NOEXCEPT { return x >= 0 && x < width && y >= 0 && y < height; }

    template<int hRadius, int vRadius>
    inline void blurGaussian(fl::alpha8 dimFactor) FL_NOEXCEPT {
        gfx::blurGaussian<hRadius, vRadius>(*this, dimFactor);
    }

    template<int hRadius, int vRadius>
    inline void blurGaussian(fl::alpha16 dimFactor) FL_NOEXCEPT {
        gfx::blurGaussian<hRadius, vRadius>(*this, dimFactor);
    }

    template<int hRadius, int vRadius>
    inline void blurGaussian() FL_NOEXCEPT {
        gfx::blurGaussian<hRadius, vRadius>(*this);
    }

    template<typename Coord>
    inline void drawLine(const RGB_T& color, Coord x0, Coord y0, Coord x1, Coord y1,
                         fl::DrawMode mode = fl::DrawMode::DRAW_MODE_BLEND) FL_NOEXCEPT {
        gfx::drawLine(*this, color, x0, y0, x1, y1, mode);
    }

    template<typename Coord>
    inline void drawDisc(const RGB_T& color, Coord cx, Coord cy, Coord r,
                         fl::DrawMode mode = fl::DrawMode::DRAW_MODE_BLEND) FL_NOEXCEPT {
        gfx::drawDisc(*this, color, cx, cy, r, mode);
    }

    template<typename Coord>
    inline void drawRing(const RGB_T& color, Coord cx, Coord cy, Coord r, Coord thickness,
                         fl::DrawMode mode = fl::DrawMode::DRAW_MODE_BLEND) FL_NOEXCEPT {
        gfx::drawRing(*this, color, cx, cy, r, thickness, mode);
    }

    template<typename Coord>
    inline void drawStrokeLine(const RGB_T& color, Coord x0, Coord y0, Coord x1, Coord y1, Coord thickness,
                               LineCap cap = LineCap::FLAT, fl::DrawMode mode = fl::DrawMode::DRAW_MODE_BLEND) FL_NOEXCEPT {
        gfx::drawStrokeLine(*this, color, x0, y0, x1, y1, thickness, cap, mode);
    }
};

}  // namespace gfx
}  // namespace fl
