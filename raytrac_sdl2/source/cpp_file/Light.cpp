#include "Light.h"

// Default constructor implementation
Light::Light() : position(Vec3SIMD()), intensity(Vec3SIMD()), direction(Vec3SIMD()), u(Vec3SIMD()), v(Vec3SIMD()), width(0), height(0) {}
 // Varsayýlan constructor eklendi
//Light::Light() : position(0, 0, 0), intensity(1, 1, 1), direction(0, 1, 0), u(1, 0, 0), v(0, 0, 1), width(1), height(1) {}
// You can add other member function implementations if needed
