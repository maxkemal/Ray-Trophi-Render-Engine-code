#ifndef AABB_H
#define AABB_H

#include "Vec3SIMD.h"
#include "Ray.h"

class AABB {
public:
    Vec3SIMD min, max;
   
    AABB() {}
    AABB(const Vec3SIMD& a, const Vec3SIMD& b) : min(a), max(b) {}
    bool overlaps(const AABB& other) const {
        return (min.x() <= other.max.x() && max.x() >= other.min.x()) &&
            (min.y() <= other.max.y() && max.y() >= other.min.y()) &&
            (min.z() <= other.max.z() && max.z() >= other.min.z());
    }

    bool hit(const Ray& r, double t_min, double t_max) const {
        for (int a = 0; a < 3; a++) {
            auto invD = 1.0f / r.direction.get(a);
            auto t0 = (min.get(a) - r.origin.get(a)) * invD;
            auto t1 = (max.get(a) - r.origin.get(a)) * invD;
            if (invD < 0.0f)
                std::swap(t0, t1);
            t_min = t0 > t_min ? t0 : t_min;
            t_max = t1 < t_max ? t1 : t_max;
            if (t_max <= t_min)
                return false;
        }
        return true;
    }

    Vec3SIMD getPositiveVertex(const Vec3SIMD& normal) const {
        return Vec3SIMD(
            normal.x() >= 0 ? max.x() : min.x(),
            normal.y() >= 0 ? max.y() : min.y(),
            normal.z() >= 0 ? max.z() : min.z()
        );
    }
    double surface_area() const;
private:
   
    
};

AABB surrounding_box(const AABB& box0, const AABB& box1);

#endif // AABB_H
