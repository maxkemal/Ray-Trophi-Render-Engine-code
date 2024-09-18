#include "Volumetric.h"
#include "Vec3.h"
#include "Ray.h"
#include <cmath>

Volumetric::Volumetric(const Vec3& a, double d, double ap, double max_d, const Vec3& e)
    : albedo(a), density(d), absorption_probability(ap), max_distance(max_d),emission(e) {}

Vec3SIMD Volumetric::emitted(double u, double v, const Vec3SIMD& p) const {
    return 5*emission * albedo *calculate_density(p.length()); // Hacimde yoðunlukla deðiþen ýþýk emisyonu

}

bool Volumetric::scatter(const Ray& r_in, const HitRecord& rec, Vec3SIMD& attenuation, Ray& scattered) const {
    double random = random_double();

    // Distance-based density and absorption calculation
    double distance_to_center = (rec.point - Vec3(0, 0, 0)).length(); // Assuming center is at origin
    double local_density = calculate_density(distance_to_center);
    double local_absorption = calculate_absorption(distance_to_center);

    // Absorption check
    if (random < local_absorption) {
        attenuation = Vec3(0, 0, 0); // Complete absorption
        return false;
    }
    // Saçýlma yönünü hesapla (Henyey-Greenstein phase function)
    double g = 0.7;
    Vec3 scatter_direction = r_in.direction + sample_henyey_greenstein(r_in.direction, g);
    // Scattering
    //Vec3 scatter_direction = r_in.direction + Vec3::random_in_unit_sphere() * local_density;
    scattered = Ray(rec.point, scatter_direction.normalize());
    attenuation = albedo; // Constant albedo for now

    return true;
}

double Volumetric::calculate_density(double distance_to_center) const {
    return std::max(0.0, density * (1.0 - distance_to_center / max_distance));
}

double Volumetric::calculate_absorption(double distance_to_center) const {
    double normalized_distance = distance_to_center / max_distance;
    return std::min(1.0, absorption_probability * (1.0 - normalized_distance));
}
Vec3 Volumetric::random_in_unit_sphere() const {
    while (true) {
        Vec3 p = Vec3(random_double(-1, 1), random_double(-1, 1), random_double(-1, 1));
        if (p.length_squared() < 1) return p;
    }
}


Vec3 Volumetric::sample_henyey_greenstein(const Vec3& wi, double g) const {
    double cos_theta = 1.0 - 2.0 * random_double();
    if (std::abs(g) > 0.001) {
        cos_theta = (1.0 + g * g - std::pow((1.0 - g * g) / (1.0 + g * (2.0 * random_double() - 1.0)), 2)) / (2.0 * g);
    }
    double sin_theta = std::sqrt(1.0 - cos_theta * cos_theta);
    double phi = 2.0 * M_PI * random_double();

    Vec3 u = Vec3::random_unit_vector().cross(wi).normalize();
    Vec3 v = wi.cross(u);

    return (u * std::cos(phi) * sin_theta + v * std::sin(phi) * sin_theta + wi * cos_theta).normalize();
}