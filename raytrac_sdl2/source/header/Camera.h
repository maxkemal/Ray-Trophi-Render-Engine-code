#ifndef CAMERA_H
#define CAMERA_H

#include <vector>
#include "Vec3.h"
#include "Matrix4x4.h"
#include "Ray.h"
#include "AABB.h"
#include "ThreadLocalRNG.h"
class Camera {
private:
    struct Plane {
        Vec3SIMD normal;
        double distance;

        Plane() : normal(Vec3SIMD()), distance(0) {}
        Plane(const Vec3SIMD& n, const Vec3& point) : normal(n.normalize()) {
            distance = -Vec3::dot(normal, point);
        }

        double distanceToPoint(const Vec3& point) const {
            return Vec3::dot(normal, point) + distance;
        }
    };

public:
    int blade_count;
    double aperture;
    double focus_dist;
    Vec3 origin;
    Vec3 u, v, w;
    Vec3 lookfrom;
    Vec3 lookat;
    Vec3 vup;
    double aspect;
    double near_dist;
    double far_dist;
    double fov;
    double aspect_ratio;
    double vfov;
   // Camera(Vec3 lookfrom, Vec3 lookat, Vec3 vup, double vfov, double aspect, double aperture, double focus_dist);

    Camera(Vec3 lookfrom, Vec3 lookat, Vec3 vup, double vfov, double aspect, double aperture, double focus_dist, int blade_count);
    Camera();
    Ray get_ray(double s, double t) const;

    Vec3 random_in_unit_polygon(int sides) const;

    double calculate_bokeh_intensity(const Vec3SIMD& point) const;

    Vec3SIMD create_bokeh_shape(const Vec3SIMD& color, double intensity) const;

    bool isPointInFrustum(const Vec3SIMD& point, double size) const;
     Matrix4x4 getRotationMatrix() const ;
    bool isAABBInFrustum(const AABB& aabb) const;
    std::vector<AABB> performFrustumCulling(const std::vector<AABB>& objects) const;

private:
    void updateFrustumPlanes();

    Vec3SIMD getViewDirection() const;

    
    Vec3 lower_left_corner;
    Vec3 horizontal;
    Vec3 vertical;
   
    double lens_radius;

    // Frustum culling için ek alanlar

    Plane frustum_planes[6];
};

#endif // CAMERA_H
