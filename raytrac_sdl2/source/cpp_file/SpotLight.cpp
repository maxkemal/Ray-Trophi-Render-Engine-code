#include "SpotLight.h"
#include <cmath>

SpotLight::SpotLight(const Vec3SIMD& pos, const Vec3SIMD& dir, const Vec3SIMD& intens, float ang, float rad)
    : angle_degrees(ang), radius(rad) {
    position = pos;
    direction = dir.normalize();
    intensity = intens;
}

Vec3SIMD SpotLight::getDirection(const Vec3SIMD& point) const {
    // Calculate the direction from the light to the point
    return (point - position).normalize();
}

Vec3SIMD SpotLight::getIntensity(const Vec3SIMD& point) const {
    // Calculate the cosine of the angle between the light direction and the direction to the point
    float cos_theta = direction.dot (getDirection(point));

    // Check if the point is within the spotlight's cone
    if (cos_theta > std::cos(angle_degrees)) {
        // Intensity falls off as a function of the cosine of the angle
        float falloff = std::pow(cos_theta, 2.0f);  // You can adjust the exponent for different falloff rates
        return intensity * falloff;
    }
    else {
        // Outside the spotlight's cone, no intensity
        return Vec3SIMD(0.0, 0.0, 0.0);
    }
}

Vec3SIMD SpotLight::random_point() const {
    // Generate a random point within the sphere of radius `radius` around the position
    Vec3SIMD random_offset = Vec3SIMD::random_in_unit_sphere() * radius;
    return position + random_offset;
}

LightType SpotLight::type() const {
    return LightType::Spot;
}
