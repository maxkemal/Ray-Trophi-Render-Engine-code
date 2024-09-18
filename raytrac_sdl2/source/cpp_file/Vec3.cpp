#include "Vec3.h"
#include "Vec3SIMD.h"
#include <cstdlib> 
#include <limits>  
#include <random>
Vec3::Vec3() : x(0), y(0), z(0) {}

Vec3::Vec3(double x, double y, double z) : x(x), y(y), z(z) {}

double Vec3::operator[](int index) const {
    if (index == 0) return x;
    else if (index == 1) return y;
    else return z;
}
Vec3::Vec3(double value) : x(value), y(value), z(value) {}
double& Vec3::operator[](int index) {
    if (index == 0) return x;
    else if (index == 1) return y;
    else return z;
}



Vec3 Vec3::operator*(const Vec3& other) const {
    // Implementation of the operator
    return Vec3(x * other.x, y * other.y, z * other.z);
}
Vec3& Vec3::operator*=(double t) {
    x *= t;
    y *= t;
    z *= t;
    return *this;
}

Vec3& Vec3::operator*=(const Vec3& other) {
    x *= other.x;
    y *= other.y;
    z *= other.z;
    return *this;
}
Vec3 Vec3::operator-() const {
    return Vec3(-x, -y, -z);
}

Vec3 Vec3::operator+(const Vec3& other) const {
    return Vec3(x + other.x, y + other.y, z + other.z);
}

Vec3 Vec3::operator-(const Vec3& other) const {
    return Vec3(x - other.x, y - other.y, z - other.z);
}

Vec3 Vec3::operator*(double t) const {
    return Vec3(x * t, y * t, z * t);
}

Vec3& Vec3::operator+=(const Vec3& other) {
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
}

Vec3& Vec3::operator/=(double t) {
    x /= t;
    y /= t;
    z /= t;
    return *this;
}

double Vec3::length() const {
    return sqrt(length_squared());
}

double Vec3::length_squared() const {
    return x * x + y * y + z * z;
}

Vec3 Vec3::normalize() const {
    double len_sq = length_squared();
    if (len_sq > 1e-10) { // Küçük bir epsilon deðeri
        double len = std::sqrt(len_sq);
        return Vec3(x / len, y / len, z / len);
    }
    return Vec3(0, 0, 0); // Vektör uzunluðu sýfýrsa, sýfýr vektör döndür
}

// Mersenne Twister rastgele sayý üreteci
std::mt19937 rng;
std::uniform_real_distribution<double> dist(0.0, 1.0);
Vec3 Vec3::random_cosine_direction(const Vec3& normal) {
    // Rastgele bir nokta üretmek için küresel koordinatlar kullanýyoruz.
    double r1 = random_double();
    double r2 = random_double();
    double z = sqrt(1.0f - r2);

    double phi = 2 * M_PI * r1;
    double x = cos(phi) * sqrt(r2);
    double y = sin(phi) * sqrt(r2);

    Vec3 random_dir(x, y, z);

    // Yukarý vektörüyle ayný olan ortogonal bir temel oluþturun.
    Vec3 u, v;
    if (fabs(normal.x) > 0.1f) {
        u = Vec3(0.0f, 1.0f, 0.0f).cross(normal).normalize();
    }
    else {
        u = Vec3(1.0f, 0.0f, 0.0f).cross(normal).normalize();
    }
    v = normal.cross(u);

    // Koordinat sistemini kullanarak yön vektörünü oluþturun.
    Vec3SIMD weighted_dir = u * random_dir.x + v * random_dir.y + normal * random_dir.z;

    return weighted_dir.normalize(); // Normalize edilmiþ yön vektörü
}


Vec3 Vec3::random_in_hemisphere(const Vec3& normal) {
    Vec3 inUnitSphere = random_in_unit_sphere();
    if (Vec3::dot(inUnitSphere, normal) > 0.0f) // In the same hemisphere as the normal
        return inUnitSphere;
    else
        return -inUnitSphere;
}
// Rastgele sayý üreteciyi tohumla
void seed_random() {
    std::random_device rd;
    rng.seed(rd());
}
// [0, 1) aralýðýnda rastgele bir sayý üret
double random_double() {
    return dist(rng);
}
Vec3 Vec3::random(double min, double max) {
    return Vec3(random_double(min, max), random_double(min, max), random_double(min, max));
}

Vec3 Vec3::random_in_unit_disk() {
    while (true) {
        auto p = Vec3(random_double(-1, 1), random_double(-1, 1), 0);
        if (p.length_squared() >= 1) continue;
        return p;
    }
}

Vec3 Vec3::random_in_unit_sphere() {
    while (true) {
        auto p = Vec3::random(-1, 1);
        if (p.length_squared() >= 1) continue;
        return p;
    }
}
Vec3 unit_vector(const Vec3& v) {
    return v / v.length();
}
double random_double(double min, double max) {
    // Return a random real in [min,max)
    return min + (max - min) * random_double();
}

Vec3 Vec3::random_unit_vector() {
    auto a = random_double(0, 2 * M_PI);
    auto z = random_double(-1, 1);
    auto r = sqrt(1 - z * z);
    return Vec3(r * cos(a), r * sin(a), z);
}

Vec3 Vec3::reflect(const Vec3& v, const Vec3& n) {
    return v - 2 * Vec3::dot(v, n) * n;
}

Vec3 Vec3::refract(const Vec3& uv, const Vec3& n, double etai_over_etat) {
    auto cos_theta = fmin(Vec3::dot(-uv, n), 1.0);
    Vec3 r_out_perp = etai_over_etat * (uv + cos_theta * n);
    Vec3 r_out_parallel = -sqrt(fabs(1.0 - r_out_perp.length_squared())) * n;
    return r_out_perp + r_out_parallel;
}



Vec3 Vec3::cross(const Vec3& v1, const Vec3& v2) {
    return Vec3(v1.y * v2.z - v1.z * v2.y,
        v1.z * v2.x - v1.x * v2.z,
        v1.x * v2.y - v1.y * v2.x);
}

Vec3 Vec3::operator/(double t) const {
    return *this * (1.0 / t);
}
Vec3 Vec3::min(const Vec3& a, const Vec3& b) {
    return Vec3(
        std::min(a.x, b.x),
        std::min(a.y, b.y),
        std::min(a.z, b.z)
    );
}

Vec3 Vec3::max(const Vec3& a, const Vec3& b) {
    return Vec3(
        std::max(a.x, b.x),
        std::max(a.y, b.y),
        std::max(a.z, b.z)
    );
}

Vec3 operator*(double t, const Vec3& v) {
    return Vec3(v.x * t, v.y * t, v.z * t);
}

double Vec3::dot(const Vec3& v1, const Vec3& v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}
