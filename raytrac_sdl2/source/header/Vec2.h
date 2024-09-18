#ifndef VEC2_H
#define VEC2_H

#include <iostream>

class Vec2 {
public:
    union {
        struct {
            double x, y;
        };
        struct {
            double u, v;
        };
    };

    // Constructors
    Vec2(double u = 0.0, double v = 0.0);
    Vec2(const Vec2& other);
   
    // Common vector operations
    double length() const;
    double lengthSquared() const;
    void normalize();
    Vec2 normalized() const;
    Vec2 rotate(double angle) const;

    // Dot and cross products
    double dot(const Vec2& other) const;
    double cross(const Vec2& other) const;

    // Angle between vectors
    double angle(const Vec2& other) const;

    // Operator overloads
    Vec2 operator+(const Vec2& other) const;
    Vec2 operator-(const Vec2& other) const;
    Vec2 operator*(double scalar) const;
    Vec2 operator/(double scalar) const;
    Vec2& operator+=(const Vec2& other);
    Vec2& operator-=(const Vec2& other);
    Vec2& operator*=(double scalar);
    Vec2& operator/=(double scalar);
    bool operator==(const Vec2& other) const;
    bool operator!=(const Vec2& other) const;
    Vec2 operator-() const;
    double& operator[](int i);
    const double& operator[](int i) const;
    // Vekt�r ile b�lme operat�r�
   
  

    Vec2 operator/(const Vec2& other) const {
        return Vec2(x / other.x, y / other.y);
    }
    // Static methods for common vectors
    static Vec2 zero();
    static Vec2 one();
    static Vec2 up();
    static Vec2 down();
    static Vec2 left();
    static Vec2 right();

    // Utility functions
    void clamp(double minVal, double maxVal);
    Vec2 lerp(const Vec2& other, double t) const;
    static Vec2 min(const Vec2& a, const Vec2& b) {
        return Vec2(std::min(a.u, b.u), std::min(a.v, b.v));
    }

    static Vec2 max(const Vec2& a, const Vec2& b) {
        return Vec2(std::max(a.u, b.u), std::max(a.v, b.v));
    }
    // Stream insertion operator for easy printing
    friend std::ostream& operator<<(std::ostream& os, const Vec2& vec);
};

// Non-member operator overloads
Vec2 operator*(double scalar, const Vec2& vec);

#endif // VEC2_H