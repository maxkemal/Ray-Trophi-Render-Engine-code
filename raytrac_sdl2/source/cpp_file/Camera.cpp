#include "Camera.h"
#include <cmath>
#include <stdlib.h>
#include "Matrix4x4.h"
Camera::Camera(Vec3 lookfrom, Vec3 lookat, Vec3 vup, double vfov, double aspect, double aperture, double focus_dist, int blade_count) {
    double theta = vfov * M_PI / 180;
    double half_height = tan(theta / 2);
    double half_width = aspect * half_height;
    origin = lookfrom;
    w = (lookfrom - lookat).normalize();
    u = Vec3::cross(vup, w).normalize();
    v = Vec3::cross(w, u);
    lower_left_corner = origin - half_width * focus_dist * u - half_height * focus_dist * v - focus_dist * w;
    horizontal = 2 * half_width * focus_dist * u;
    vertical = 2 * half_height * focus_dist * v;
    lens_radius = aperture / 2;

    // Frustum culling için ek alanlar
    near_dist = 0.1;  // Yakýn düzlem mesafesi, ihtiyaca göre ayarlayýn
    far_dist = focus_dist * 2;  // Uzak düzlem mesafesi, ihtiyaca göre ayarlayýn
    fov = vfov;
    aspect_ratio = aspect;
    updateFrustumPlanes();
    this->aperture = aperture;
    this->blade_count = blade_count; // Býçak sayýsýný sakla
}
Camera::Camera() {

}
Ray Camera::get_ray(double s, double t) const {
    Vec3 rd = lens_radius * random_in_unit_polygon(blade_count);
    Vec3 offset = u * rd.x + v * rd.y;
    return Ray(origin + offset, lower_left_corner + s * horizontal + t * vertical - origin - offset);
}

Vec3 Camera::random_in_unit_polygon(int sides) const {
    double angle = random_double() * 2 * M_PI;
    double radius = std::pow(random_double(), 1.0 / 3.0); // Daha belirgin kenarlar için üslü daðýlým

    double step = 2 * M_PI / sides;
    double sector = angle / step;
    double fractional = sector - floor(sector);

    // Köþelere doðru daha fazla yoðunlaþma
    fractional = std::pow(fractional, 0.5);

    double x = radius * cos(step * floor(sector) + fractional * step);
    double y = radius * sin(step * floor(sector) + fractional * step);

    return Vec3SIMD(x, y, 0);
}

// Yeni fonksiyon: Bokeh þiddetini hesapla
double Camera::calculate_bokeh_intensity(const Vec3SIMD& point) const {
    double distance = (point - origin).length();
    double focal_plane_distance = focus_dist;
    double blur_factor = std::abs(distance - focal_plane_distance) / focal_plane_distance;
    return std::min(1.0, blur_factor * aperture);
}

// Iþýk kaynaklarý için özel bokeh þekli oluþtur
Vec3SIMD Camera::create_bokeh_shape(const Vec3SIMD& color, double intensity) const {
    Vec3SIMD bokeh_color = color * intensity;
    Vec3SIMD shape = random_in_unit_polygon(blade_count);
    return bokeh_color * (shape * 0.5 + Vec3SIMD(0.5, 0.5, 0.5));
}

bool Camera::isPointInFrustum(const Vec3SIMD& point, double size) const {
    for (const auto& plane : frustum_planes) {
        if (plane.distanceToPoint(point) < -size) {
            return false;  // Point is outside the frustum
        }
    }
    return true;  // Point is inside or intersects the frustum
}

void Camera::updateFrustumPlanes() {
    // Frustum düzlemlerini hesapla
    Vec3SIMD fc = origin - w * far_dist;
    float near_height = 2 * tan(fov * 0.5f * M_PI / 180) * near_dist;
    float far_height = 2 * tan(fov * 0.5f * M_PI / 180) * far_dist;
    float near_width = near_height * aspect_ratio;
    float far_width = far_height * aspect_ratio;

    Vec3SIMD ntl = origin - w * near_dist - u * (near_width * 0.5f) + v * (near_height * 0.5f);
    Vec3SIMD ntr = origin - w * near_dist + u * (near_width * 0.5f) + v * (near_height * 0.5f);
    Vec3SIMD nbl = origin - w * near_dist - u * (near_width * 0.5f) - v * (near_height * 0.5f);
    Vec3SIMD ftr = fc + u * (far_width * 0.5f) + v * (far_height * 0.5f);

    frustum_planes[0] = Plane(Vec3SIMD::cross(ntl - ntr, ntl - ftr).normalize(), ntl);  // top
    frustum_planes[1] = Plane(Vec3SIMD::cross(nbl - ntl, nbl - fc).normalize(), nbl);   // left
    frustum_planes[2] = Plane(w, origin - w * near_dist);                           // near
    frustum_planes[3] = Plane(Vec3SIMD::cross(ntr - ntl, ntr - fc).normalize(), ntr);   // right
    frustum_planes[4] = Plane(Vec3SIMD::cross(nbl - ntr, nbl - fc).normalize(), nbl);   // bottom
    frustum_planes[5] = Plane(-w, fc);                                              // far
}
Vec3SIMD Camera::getViewDirection() const {
    return -w;  // Kameranýn baktýðý yön w vektörünün tersidir
}

Matrix4x4 Camera::getRotationMatrix() const {
    Matrix4x4 rotationMatrix;
    rotationMatrix.m[0][0] = u.x; rotationMatrix.m[0][1] = u.y; rotationMatrix.m[0][2] = u.z; rotationMatrix.m[0][3] = 0;
    rotationMatrix.m[1][0] = v.x; rotationMatrix.m[1][1] = v.y; rotationMatrix.m[1][2] = v.z; rotationMatrix.m[1][3] = 0;
    rotationMatrix.m[2][0] = w.x; rotationMatrix.m[2][1] = w.y; rotationMatrix.m[2][2] = w.z; rotationMatrix.m[2][3] = 0;
    rotationMatrix.m[3][0] = 0;   rotationMatrix.m[3][1] = 0;   rotationMatrix.m[3][2] = 0;   rotationMatrix.m[3][3] = 1;
    return rotationMatrix;
}
bool Camera::isAABBInFrustum(const AABB& aabb) const {
    for (const auto& plane : frustum_planes) {
        if (plane.distanceToPoint(aabb.getPositiveVertex(plane.normal)) < 0) {
            return false;  // AABB frustum dýþýnda
        }
    }
    return true;  // AABB frustum içinde
}

std::vector<AABB> Camera::performFrustumCulling(const std::vector<AABB>& objects) const {
    std::vector<AABB> visibleObjects;
    for (const auto& obj : objects) {
        if (isAABBInFrustum(obj)) {
            visibleObjects.push_back(obj);
        }
    }
    return visibleObjects;
}
