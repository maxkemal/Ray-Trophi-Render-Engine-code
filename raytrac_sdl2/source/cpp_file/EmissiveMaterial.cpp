#include "EmissiveMaterial.h"

EmissiveMaterial::EmissiveMaterial(const Vec3& emit)
    : emission(emit) {}

bool EmissiveMaterial::scatter(const Ray& r_in, const HitRecord& rec, Vec3SIMD& attenuation, Ray& scattered) const {
    return false; // Emissive materials do not scatter light
}

Vec3SIMD EmissiveMaterial::emitted(double u, double v, const Vec3SIMD& p) const {
    return emission;
}
