#ifndef DIRECTIONAL_LIGHT_H
#define DIRECTIONAL_LIGHT_H

#include "Light.h"
#include "Vec3.h"
#include "Vec3SIMD.h"
#include <random>

class DirectionalLight : public Light {
public:
    DirectionalLight( const Vec3SIMD& dir, const Vec3SIMD& intens, double radius);

    Vec3SIMD getDirection(const Vec3SIMD& point) const override {
        return -direction.normalize(); // Iþýk yönünün tersi
    }

    Vec3SIMD getIntensity(const Vec3SIMD& point) const override {
        return intensity; // Mesafeden baðýmsýz
    }
    Vec3SIMD getPosition() const  {
        return position; 
    }
    Vec3SIMD random_point() const override;
    LightType type() const override;
    void setPosition(const Vec3SIMD& pos) {
        position = pos;
    }
    void setDirection(const Vec3SIMD& dir) {
        position = dir;
    }
private:
    double disk_radius; // Disk'in yarýçapý
};

#endif // DIRECTIONAL_LIGHT_H
