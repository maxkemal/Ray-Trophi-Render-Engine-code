#ifndef RAY_H
#define RAY_H

#include "Vec3SIMD.h"

class Ray {
public:
    Vec3SIMD origin;
    Vec3SIMD direction;

    Ray() {}
    Ray(const Vec3SIMD& origin, const Vec3SIMD& direction) : origin(origin), direction(direction) {}

    Vec3SIMD at(float t) const { return Vec3SIMD(origin) + t * Vec3SIMD(direction); }
};

#endif // RAY_H

