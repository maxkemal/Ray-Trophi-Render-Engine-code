#ifndef POINTLIGHT_H
#define POINTLIGHT_H

#include "Light.h"
#include "Vec3.h"

class PointLight : public Light {
public:
    PointLight() : position(0, 0, 0), intensity(1, 1, 1), radius(0) {}
    PointLight(const Vec3SIMD& pos, const Vec3SIMD& intens, float rad);
    Vec3SIMD getDirection(const Vec3SIMD& point) const override {
        return (position - point).normalize();
    }
  

    Vec3SIMD getIntensity(const Vec3SIMD& point) const override {
        float distance = (position - point).length();
        return intensity / (distance * distance);
    }
    Vec3SIMD random_point() const override;
    LightType type() const override;
    Vec3SIMD getPosition() const { return position; }
 
    Vec3SIMD getIntensity() const { return intensity; }
    float getRadius() const { return radius; }
    void setPosition(const Vec3SIMD& pos) {
        position = pos;
    }

    void setIntensity(const Vec3& intens) {
        intensity = intens;
    }

    void setRadius(float rad) {
        radius = rad;
    }
    Vec3SIMD position;
    Vec3SIMD intensity;
private:
  
    float radius;
};

#endif // POINTLIGHT_H
