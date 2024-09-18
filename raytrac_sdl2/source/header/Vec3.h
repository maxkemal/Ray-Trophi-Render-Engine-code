#ifndef VEC3_H
#define VEC3_H

#include <iostream>
#include <cmath>
#include <stdexcept>
#include <algorithm> // std::clamp için

#define M_PI 3.14159265358979323846

template <typename T>
T max(T a, T b) {
    return (a > b) ? a : b;
}



template<typename T>
T clamp(const T& value, const T& min, const T& max) {
    return std::max(min, std::min(max, value));
}

class Vec3 {
public:
    double x, y, z;
    friend std::ostream& operator<<(std::ostream& os, const Vec3& vec) {
        os << vec.x << ", " << vec.y << ", " << vec.z;
        return os;
    }
  
    // Bu tanýmlamalar, r, g, b isimleriyle ayný iþlevi görecek.
    double& r() { return x; }
    double& g() { return y; }
    double& b() { return z; }
    const float& r() const { return x; }
    const float& g() const { return y; }
    const float& b() const { return z; }
    Vec3 clamp(double min, double max) const {
        return Vec3(
            x < min ? min : (x > max ? max : x),
            y < min ? min : (y > max ? max : y),
            z < min ? min : (z > max ? max : z)
        );
    }
    // Yeni kurucu: tek float deðerle baþlatma
    Vec3(double value);
    Vec3 operator*(const Vec3& other) const;
    Vec3();
    Vec3(double x, double y, double z);

    bool near_zero() const {
        const auto s = 1e-8;
        return (fabs(x) < s) && (fabs(y) < s) && (fabs(z) < s);
    }
    Vec3 operator-(double scalar) const {
        return Vec3(x - scalar, y - scalar, z - scalar);
    }
    Vec3 operator+(double scalar) const {
        return Vec3(x + scalar, y + scalar, z + scalar);
    }
    Vec3 operator/(const Vec3& v) const {
        return Vec3(x / v.x, y / v.y, z / v.z);
    }
    Vec3 operator-() const;
    Vec3& operator+=(const Vec3& v);
    Vec3& operator*=(const double t);
    Vec3& operator/=(const double t);
    Vec3 operator+(const Vec3& v) const;
    Vec3 operator-(const Vec3& v) const;
    //Vec3 operator*(const Vec3& v) const;
    Vec3 operator*(double t) const;
    Vec3 operator/(double t) const;
   static Vec3 min(const Vec3& a, const Vec3& b);
    static Vec3 max(const Vec3& a, const Vec3& b);
    Vec3& operator*=(const Vec3& other);
    //Vec3& operator*=(const Vec3& rhs);
    double length_squared() const;
    double operator[](int i) const;
    double& operator[](int i);
   static Vec3 random_cosine_direction(const Vec3& normal);
   static  Vec3 random_in_hemisphere(const Vec3& normal) ;
    inline double dot(const Vec3& other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    Vec3 cwiseProduct(const Vec3& v) const;
    //Vec3 clamp(double min, double max) const;
    double length() const;
    Vec3 normalize() const;

    static Vec3 random();
    static Vec3 random(double min, double max);
    static Vec3 random_in_unit_sphere();
    static Vec3 random_unit_vector();
    static Vec3 reflect(const Vec3& v, const Vec3& n);
    static Vec3 refract(const Vec3& uv, const Vec3& n, double etai_over_etat);
    Vec3 random_in_unit_disk();
   

    // Baðýmsýz fonksiyon olarak unit_vector
    friend Vec3 unit_vector(const Vec3& v);
   
   
    static double dot(const Vec3& v1, const Vec3& v2);
    static Vec3 cross(const Vec3& v1, const Vec3& v2);

    friend std::ostream& operator<<(std::ostream& os, const Vec3& v);
    Vec3 cross(const Vec3& v) const {
        return Vec3(
            y * v.z - z * v.y,
            z * v.x - x * v.z,
            x * v.y - y * v.x
        );
    }
    Vec3 abs() const {
        return Vec3(std::fabs(x), std::fabs(y), std::fabs(z));
    }

     

   
};
Vec3 unit_vector(const Vec3& v);
Vec3 operator*(double t, const Vec3& v);
// Function declarations
double random_double(double min, double max);
double random_double();
Vec3 operator*(double t, const Vec3& v);
#endif // VEC3_H
