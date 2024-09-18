#include "PointLight.h"

PointLight::PointLight(const Vec3SIMD& pos, const Vec3SIMD& intens, float rad)
    : position(pos), intensity(intens), radius(rad) {}

Vec3SIMD PointLight::random_point() const {
    // Generate a random point within the sphere of radius `radius` around the position
    Vec3SIMD random_offset = intensity.random_in_unit_sphere() * radius;
    return position + random_offset;
}

LightType PointLight::type() const {
    return LightType::Point;
}
