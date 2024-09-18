#ifndef LIGHT_H
#define LIGHT_H

#include "Vec3SIMD.h"

enum class LightType {
    Point,
    Directional,
    Spot,
    Area
};

class Light {
public:
    Vec3SIMD position;
    Vec3SIMD intensity;
    Vec3SIMD direction;   
    Vec3SIMD u, v;       // Alan ����� i�in d�zlem vekt�rleri
    double width, height;  // Alan ����� i�in boyutlar

    // Default constructor
    Light();

    // Sanal y�k�c� eklendi
    virtual ~Light() = default;
    virtual Vec3SIMD getDirection(const Vec3SIMD& point) const = 0;
    virtual Vec3SIMD getIntensity(const Vec3SIMD& point) const = 0;
    virtual Vec3SIMD random_point() const = 0;
    virtual LightType type() const = 0;
};

#endif // LIGHT_H
