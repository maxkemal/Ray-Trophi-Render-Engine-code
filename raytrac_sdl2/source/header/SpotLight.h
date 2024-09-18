#ifndef SPOTLIGHT_H
#define SPOTLIGHT_H

#include "Light.h"
#include "Vec3SIMD.h"
#include "Vec3.h"
class SpotLight : public Light {
public:
    SpotLight(const Vec3SIMD& pos, const Vec3SIMD& dir, const Vec3SIMD& intens, float ang, float rad);
    Vec3SIMD position;
    float angle_degrees = 30.0f;
    float angle_radians = angle_degrees * (M_PI / 180.0f);
    float radius;
    Vec3SIMD getDirection(const Vec3SIMD& point) const override;
    Vec3SIMD getIntensity(const Vec3SIMD& point) const override;
    Vec3SIMD random_point() const override;
    LightType type() const override;

private:
   
   
    
};

#endif // SPOTLIGHT_H
