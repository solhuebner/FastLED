

#include <cmath>
#include <stdint.h>

#include "fl/math_macros.h" // if needed for MAX/MIN macros
#include "fl/namespace.h"
#include "fl/scoped_ptr.h"
#include "fl/warn.h"

#include "fl/ptr.h"
#include "fl/xymap.h"
#include "fx/fx.h"
#include "fx/fx2d.h"

namespace fl {


class WaveSimulation1D {
  public:
    // Constructor:
    //  - length: inner simulation grid length (excluding the 2 boundary cells).
    //  - courantSq: simulation speed (in float, will be stored in Q15).
    //  - dampening: exponent so that the effective damping factor is 2^(dampening).
    WaveSimulation1D(uint32_t length, float courantSq = 0.16f, int dampening = 6);
    ~WaveSimulation1D() = default;

    // Set simulation speed (courant parameter) using a float.
    void setSpeed(float something);

    // Set the dampening exponent (effective damping factor is 2^(dampening)).
    void setDampenening(int damp);

    // Get the current dampening exponent.
    int getDampenening() const;

    // Get the simulation speed as a float.
    float getSpeed() const;

    // Get the simulation value at the inner grid cell x (converted to float in the range [-1.0, 1.0]).
    float get(size_t x) const;

    int16_t geti16(size_t x) const;

    int8_t geti8(size_t x) const {
        return static_cast<int8_t>(geti16(x) >> 8);
    }

    uint8_t getu8(size_t x) const {
        int16_t value = geti16(x);
        // Rebase the range from [-32768, 32767] to [0, 65535] then extract the
        // upper 8 bits.
        return static_cast<uint8_t>(((static_cast<uint16_t>(value) + 32768)) >>
                                    8);
    }

    // Returns whether x is within the inner grid bounds.
    bool has(size_t x) const;

    // Set the value at grid cell x (expected range: [-1.0, 1.0]); the value is stored in Q15.
    void set(size_t x, float value);

    // Advance the simulation one time step.
    void update();

  private:
    uint32_t length; // Length of the inner simulation grid.
    // Two grids stored in fixed Q15 format, each with length+2 entries (including boundary cells).
    fl::scoped_array<int16_t> grid1;
    fl::scoped_array<int16_t> grid2;
    size_t whichGrid; // Indicates the active grid (0 or 1).

    int16_t mCourantSq; // Simulation speed (courant squared) stored in Q15.
    int mDampenening;    // Dampening exponent (damping factor = 2^(mDampenening)).
};

class WaveSimulation2D {
  public:
    // Constructor: Initializes the simulation with inner grid size (W x H).
    // The grid dimensions include a 1-cell border on each side.
    // Here, 'speed' is specified as a float (converted to fixed Q15)
    // and 'dampening' is given as an exponent so that the damping factor is
    // 2^dampening.
    WaveSimulation2D(uint32_t W, uint32_t H, float courantSq = 0.16f,
                     float dampening = 6.0f);
    ~WaveSimulation2D() = default;

    // Set the simulation speed (courantSq) using a float value.
    void setSpeed(float something);

    // Set the dampening factor exponent.
    // The dampening factor used is 2^(dampening).
    void setDampenening(int damp);

    // Get the current dampening exponent.
    int getDampenening() const;

    // Get the simulation speed as a float (converted from fixed Q15).
    float getSpeed() const;

    // Return the value at an inner grid cell (x,y), converted to float.
    // The value returned is in the range [-1.0, 1.0].
    float getf(size_t x, size_t y) const;

    // Return the value at an inner grid cell (x,y) as a fixed Q15 integer
    // in the range [-32768, 32767].
    int16_t geti16(size_t x, size_t y) const;

    int8_t geti8(size_t x, size_t y) const {
        return static_cast<int8_t>(geti16(x, y) >> 8);
    }

    uint8_t getu8(size_t x, size_t y) const {
        int16_t value = geti16(x, y);
        // Rebase the range from [-32768, 32767] to [0, 65535] then extract the
        // upper 8 bits.
        return static_cast<uint8_t>(((static_cast<uint16_t>(value) + 32768)) >>
                                    8);
    }

    // Check if (x,y) is within the inner grid.
    bool has(size_t x, size_t y) const;

    // Set the value at an inner grid cell using a float;
    // the value is stored in fixed Q15 format.
    // value shoudl be between -1.0 and 1.0.
    void set(size_t x, size_t y, float value);

    // Advance the simulation one time step using fixed-point arithmetic.
    void update();

    uint32_t getWidth() const { return width; }
    uint32_t getHeight() const { return height; }

  private:
    uint32_t width;  // Width of the inner grid.
    uint32_t height; // Height of the inner grid.
    uint32_t stride; // Row length (width + 2 for the borders).

    // Two separate grids stored in fixed Q15 format.
    fl::scoped_array<int16_t> grid1;
    fl::scoped_array<int16_t> grid2;

    size_t whichGrid; // Indicates the active grid (0 or 1).

    int16_t mCourantSq; // Fixed speed parameter in Q15.
    int mDampening;     // Dampening exponent; used as 2^(dampening).
};

} // namespace fl
