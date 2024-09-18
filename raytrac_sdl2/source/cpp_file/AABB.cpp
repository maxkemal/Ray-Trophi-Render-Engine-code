#include "AABB.h"
double AABB::surface_area() const {
    double x = max.x() - min.x();
    double y = max.y() - min.y();
    double z = max.z() - min.z();
    return 2 * (x * y + x * z + y * z);
}

AABB surrounding_box(const AABB& box0, const AABB& box1) {
    Vec3SIMD small(fmin(box0.min.x(), box1.min.x()),
        fmin(box0.min.y(), box1.min.y()),
        fmin(box0.min.z(), box1.min.z()));

    Vec3SIMD big(fmax(box0.max.x(), box1.max.x()),
        fmax(box0.max.y(), box1.max.y()),
        fmax(box0.max.z(), box1.max.z()));

    return AABB(small, big);
}
