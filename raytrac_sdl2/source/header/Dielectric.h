#ifndef DIELECTRIC_H
#define DIELECTRIC_H

#include "Material.h"
#include <array>
class Dielectric : public Material {
public:
    MaterialType type() const override { return MaterialType::Dielectric; }
    Dielectric(double index_of_refraction,
        const Vec3SIMD& color = Vec3SIMD(0.95, 0.95, 1.0),
        double caustic_intensity = 0.1,
        double thickness = 0.006,  // Typical car windshield thickness
        double tint_factor = 0.2,
        double scratch_density = 0.01);

    virtual Vec3SIMD emitted(double u, double v, const Vec3SIMD& p) const override;

    virtual bool scatter(const Ray& r_in, const HitRecord& rec, Vec3SIMD& attenuation, Ray& scattered) const override;
    virtual double getIndexOfRefraction() const override;

    Vec3SIMD fresnel(const Vec3SIMD& incident, const Vec3SIMD& normal, const std::array<double, 3>& ior) const;

    float get_scattering_factor() const override {
        return 0.001f; // Example value
    }
    Vec3SIMD color; // Base color of the glass
    double caustic_intensity; // Intensity of the caustic effect
    float thickness; // Thickness of the glass
    double tint_factor; // How much the glass is tinted
    double scratch_density; // Density of scratches on the glass
    std::array<double, 3> ir;  // Index of refraction for R, G, B

    double roughness;
private:
   
   
    double calculate_caustic_factor(double cos_theta, double refraction_ratio, bool is_reflected) const;
    Vec3SIMD calculate_caustic(const Vec3SIMD& incident, const Vec3SIMD& normal, const Vec3SIMD& refracted) const;
    Vec3SIMD apply_tint(const Vec3SIMD& color) const;
    Vec3SIMD apply_roughness(const Vec3SIMD& direction, const Vec3SIMD& normal) const;
    double calculate_attenuation(double distance) const;
    Vec3SIMD apply_scratches(const Vec3SIMD& color, const Vec3SIMD& point) const;
    static double reflectance(double cosine, double ref_idx);
};

#endif // DIELECTRIC_H