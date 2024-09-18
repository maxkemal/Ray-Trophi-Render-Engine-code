#ifndef AREA_LIGHT_H
#define AREA_LIGHT_H

#include "Light.h"
#include "Vec3.h"

class AreaLight : public Light {
public:
    AreaLight(const Vec3SIMD& pos, const Vec3SIMD& u_vec, const Vec3SIMD& v_vec, double w, double h, const Vec3SIMD& intens)
        : position(pos), u(u_vec), v(v_vec), width(w), height(h), intensity(intens) {
        direction = u.cross(v).normalize();
    }

    // Set metodlarý
    void setPosition(const Vec3SIMD& pos) { position = pos; }
    void setU(const Vec3SIMD& u_vec) { u = u_vec; direction = u.cross(v).normalize(); }
    void setV(const Vec3SIMD& v_vec) { v = v_vec; direction = u.cross(v).normalize(); }
    void setWidth(double w) { width = w; }
    void setHeight(double h) { height = h; }
    void setIntensity(const Vec3SIMD& intens) { intensity = intens; }

    // Override metodu
    Vec3SIMD random_point() const override {
        double rand_u = static_cast<double>(rand()) / RAND_MAX;
        double rand_v = static_cast<double>(rand()) / RAND_MAX;
        return position + u * rand_u * width + v * rand_v * height;
    }

    // Get metodlarý
    Vec3SIMD getPosition() const { return position; }
    Vec3SIMD getU() const { return u; }
    Vec3SIMD getV() const { return v; }
    double getWidth() const { return width; }
    double getHeight() const { return height; }
    Vec3SIMD getIntensity() const { return intensity; }
    Vec3SIMD getDirection(const Vec3SIMD& point) const {
        return (position - point).normalize();
    }

    Vec3SIMD getIntensity(const Vec3SIMD& point) const {
        // Assuming intensity is constant across the area light.
        return intensity;
    }

    LightType type() const override { return LightType::Area; }

private:
    Vec3SIMD position;
    Vec3SIMD u;
    Vec3SIMD v;
    double width;
    double height;
    Vec3SIMD intensity;
    Vec3SIMD direction;
};

#endif // AREA_LIGHT_H
