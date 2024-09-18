#include "Ray.h"
#include "Hittable.h"
#include <cmath>
#include "Dielectric.h"

Dielectric::Dielectric(double index_of_refraction, const Vec3SIMD& color, double caustic_intensity,
    double thickness, double tint_factor, double roughness)
    : color(color), caustic_intensity(caustic_intensity), thickness(thickness),
    tint_factor(tint_factor), roughness(roughness) {
    // Slightly different IOR for each color channel to simulate dispersion
    ir = { index_of_refraction, index_of_refraction * 1.01, index_of_refraction * 1.02 };
}

Vec3SIMD Dielectric::emitted(double u, double v, const Vec3SIMD& p) const {
    return Vec3SIMD(0.0, 0.0, 0.0);
}

bool Dielectric::scatter(const Ray& r_in, const HitRecord& rec, Vec3SIMD& attenuation, Ray& scattered) const {
    Vec3SIMD unit_direction = r_in.direction.normalize();
    Vec3SIMD outward_normal = rec.front_face ? rec.normal : -rec.normal;

    Vec3SIMD reflected = Vec3SIMD::reflect(unit_direction, outward_normal);
    Vec3SIMD refracted;
    Vec3SIMD fr = fresnel(unit_direction, outward_normal, rec.front_face ? ir : std::array<double, 3>{1.0 / ir[0], 1.0 / ir[1], 1.0 / ir[2]});

    if (fr.x() < Vec3SIMD::random_double()) {
        refracted = Vec3SIMD::refract(unit_direction, outward_normal, rec.front_face ? 1.0 / ir[0] : ir[0]);
    }
    else {
        refracted = reflected;
    }

    Vec3SIMD direction = apply_roughness(refracted, outward_normal);
    Vec3SIMD offset = Vec3SIMD(outward_normal * thickness) * Vec3SIMD(rec.front_face ? -1 : 1);
    scattered = Ray(rec.point + offset, direction);

    Vec3SIMD caustic_color = calculate_caustic(unit_direction, outward_normal, refracted);
    attenuation = color + caustic_color;
    attenuation = apply_tint(attenuation);
    attenuation = attenuation * calculate_attenuation(thickness);

    return true;
}

double Dielectric::getIndexOfRefraction() const {
    return ir[1];  // Return the green channel IOR as an average
}

Vec3SIMD Dielectric::fresnel(const Vec3SIMD& incident, const Vec3SIMD& normal, const std::array<double, 3>& ior) const {
    float cos_i = std::clamp(Vec3SIMD::dotfloat(-incident, normal), -1.0f, 1.0f);
    float sin_t2 = ior[1] * ior[1] * (1.0 - cos_i * cos_i);

    if (sin_t2 > 1.0) return Vec3SIMD(1.0, 1.0, 1.0);  // Total internal reflection

    float cos_t = std::sqrt(1.0 - sin_t2);
    float r_ortho = (ior[1] * cos_i - cos_t) / (ior[1] * cos_i + cos_t);
    float r_para = (cos_i - ior[1] * cos_t) / (cos_i + ior[1] * cos_t);

    float fr = (r_ortho * r_ortho + r_para * r_para) / 2.0;

    // Fresnel etkisine roughness dahil ediliyor
    float fresnel_factor = fr * (1.0 - roughness) + roughness;  // Pürüzlülük etkisi dahil ediliyor

    return Vec3SIMD(fresnel_factor, fresnel_factor, fresnel_factor);
}


Vec3SIMD Dielectric::calculate_caustic(const Vec3SIMD& incident, const Vec3SIMD& normal, const Vec3SIMD& refracted) const {
    float dot_product = Vec3SIMD::dotfloat(incident, normal);

    float refraction_angle = std::acos(std::clamp(Vec3SIMD::dotfloat(refracted, -normal), -1.0f, 1.0f));
    float caustic_factor = std::pow(std::sin(refraction_angle), 2) * caustic_intensity;
    return Vec3SIMD(1.0, 1.0, 0.9) * caustic_factor * (1.0 - std::abs(dot_product));
}

Vec3SIMD Dielectric::apply_tint(const Vec3SIMD& color) const {
    Vec3SIMD tint_color(0.8, 0.8, 0.85);  // Daha nötr bir tonlama, mavi tonu hafifletildi
    return color * (1.0 - tint_factor) + tint_color * tint_factor;
}

Vec3SIMD Dielectric::apply_roughness(const Vec3SIMD& direction, const Vec3SIMD& normal) const {
    // Eðer roughness çok küçükse yönü deðiþtirme
    if (roughness < 0.0001f) return direction;

    Vec3SIMD random_vec = Vec3SIMD::random_in_unit_sphere();
    Vec3SIMD tangent = Vec3SIMD::cross(normal, Vec3SIMD(0, 1, 0));

    // Eðer tangent vektörü çok küçükse farklý bir temel oluþtur
    if (tangent.length() < 0.001) tangent = Vec3SIMD::cross(normal, Vec3SIMD(1, 0, 0));

    Vec3SIMD bitangent = Vec3SIMD::cross(normal, tangent);

    // Roughness vektörünün etkisini sýnýrlamak için scale kullanýyoruz
    float scale = roughness;
    Vec3SIMD roughness_effect = (random_vec.x() * tangent + random_vec.y() * bitangent + random_vec.z() * normal) * scale;

    // Normalize edilmiþ yön vektörü döndürülüyor
    return (direction + roughness_effect).normalize();
}

double Dielectric::calculate_attenuation(double distance) const {
    const float absorption_coefficient = 0.1;  // Daha düþük bir deðer
    return std::exp(-absorption_coefficient * distance);
}