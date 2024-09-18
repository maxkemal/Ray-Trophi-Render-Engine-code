#include "DiffuseLight.h"

DiffuseLight::DiffuseLight(Vec3SIMD c)
    : emit(c) {}

bool DiffuseLight::scatter(const Ray& r_in, const HitRecord& rec, Vec3SIMD& attenuation, Ray& scattered) const {
    return false; // DiffuseLight materials do not scatter light
}

Vec3SIMD DiffuseLight::emitted(double u, double v, const Vec3SIMD& p) const {
    return emit;
}
