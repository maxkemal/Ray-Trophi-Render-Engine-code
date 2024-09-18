#ifndef DIFFUSELIGHT_H
#define DIFFUSELIGHT_H

#include "Material.h"

class DiffuseLight : public Material {
public:
    DiffuseLight(Vec3SIMD c);
   
    virtual bool scatter(const Ray& r_in, const HitRecord& rec, Vec3SIMD& attenuation, Ray& scattered) const override;

    virtual Vec3SIMD emitted(double u, double v, const Vec3SIMD& p) const override;
   
public:
    Vec3 emit;
};

#endif // DIFFUSELIGHT_H
