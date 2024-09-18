#include "Triangle.h"
#include "Ray.h"
#include "AABB.h"
#include <cmath>
#include <algorithm>
#include <iostream>
#include "globals.h"
#include "PrincipledBSDF.h"
#include <chrono>

Triangle::Triangle()
    : smoothGroup(0) {}

Triangle::Triangle(const Vec3SIMD& a, const Vec3SIMD& b, const Vec3SIMD& c, std::shared_ptr<Material> m)
    : v0(a), v1(b), v2(c), material(m), smoothGroup(0) {
    initialize_transforms();  // Transform iþlemlerini baþlat
}

Triangle::Triangle(const Vec3SIMD& a, const Vec3SIMD& b, const Vec3SIMD& c,
    const Vec3SIMD& na, const Vec3SIMD& nb, const Vec3SIMD& nc,
    const Vec2& ta, const Vec2& tb, const Vec2& tc,
    std::shared_ptr<Material> m, int sg)
    : v0(a), v1(b), v2(c),
    n0(na), n1(nb), n2(nc),
    t0(ta), t1(tb), t2(tc),
    material(m), smoothGroup(sg) {
    update_bounding_box();
    initialize_transforms();  // Transform iþlemlerini baþlat
}

void Triangle:: setUVCoordinates(const Vec2& t0, const Vec2& t1, const Vec2& t2) {
    this->t0 = t0;
    this->t1 = t1;
    this->t2 = t2;
}

std::tuple<Vec2, Vec2, Vec2> Triangle::getUVCoordinates() const {
    return std::make_tuple(t0, t1, t2);
}
void Triangle::set_normals(const Vec3SIMD& normal0, const Vec3SIMD& normal1, const Vec3SIMD& normal2) {
    n0 = normal0;
    n1 = normal1;
    n2 = normal2;
}

void Triangle::set_transform(const Matrix4x4& t) {
    transform = t;

    update_bounding_box();
}
void Triangle::updateTriangleTransform(Triangle& triangle, const Matrix4x4& transform) {
    triangle.set_transform(transform);
}

void Triangle::update_bounding_box() {
    Vec3SIMD transformed_v0 = transform.transform_point(v0);
    Vec3SIMD transformed_v1 = transform.transform_point(v1);
    Vec3SIMD transformed_v2 = transform.transform_point(v2);

    min_point = Vec3SIMD(
        std::min({ transformed_v0.x(), transformed_v1.x(), transformed_v2.x()}),
        std::min({ transformed_v0.y(), transformed_v1.y(), transformed_v2.y() }),
        std::min({ transformed_v0.z(), transformed_v1.z(), transformed_v2.z() })
    );
    max_point = Vec3SIMD(
        std::max({ transformed_v0.x(), transformed_v1.x(), transformed_v2.x() }),
        std::max({ transformed_v0.y(), transformed_v1.y(), transformed_v2.y() }),
        std::max({ transformed_v0.z(), transformed_v1.z(), transformed_v2.z() })
    );
}
double safeAcos(double x) {
    if (x < -1.0) x = -1.0;
    else if (x > 1.0) x = 1.0;
    return std::acos(x);
}
Vec3SIMD Triangle::calculateBarycentricCoordinates(const Vec3SIMD& point) const {
    // Üçgen kenarlarý arasýndaki vektörler
    const Vec3SIMD v0v1 = v1 - v0;
    const Vec3SIMD v0v2 = v2 - v0;
    const Vec3SIMD p = point - v0;

    // Daha önce tekrar eden hesaplamalar azaltýldý
    const float d00 = Vec3SIMD::dotfloat(v0v1, v0v1);
    const float d01 = Vec3SIMD::dotfloat(v0v1, v0v2);
    const float d11 = Vec3SIMD::dotfloat(v0v2, v0v2);
    const float d20 = Vec3SIMD::dotfloat(p, v0v1);
    const float d21 = Vec3SIMD::dotfloat(p, v0v2);

    // Determinant ve barycentrik koordinatlar
    const float denom = 1.0 / (d00 * d11 - d01 * d01); // Tersini almak yerine burada çarpým daha verimli
    const float v = (d11 * d20 - d01 * d21) * denom;
    const float w = (d00 * d21 - d01 * d20) * denom;
    const float u = 1.0 - v - w;

    return Vec3SIMD(u, v, w);
}


// Eþik deðeri bir sabit olarak hesaplayýn
const double cos_threshold = std::cos(60.0 * M_PI / 180.0);
bool Triangle::hit(const Ray& r, double t_min, double t_max, HitRecord& rec) const {
    // Vertexleri dönüþtürme
    const Vec3SIMD edge1 = transformed_v1 - transformed_v0;
    const Vec3SIMD edge2 = transformed_v2 - transformed_v0;

    Vec3SIMD h = Vec3SIMD::cross(r.direction, edge2);
    float a = Vec3SIMD::dotfloat(edge1, h);
    double adaptive_epsilon = std::max(1e-6, 1e-3 * std::max(edge1.length(), edge2.length()));
    adaptive_epsilon = std::clamp(adaptive_epsilon, 1e-6, 1e-3);

    if (std::abs(a) < adaptive_epsilon)
        return false;

    float f = 1.0 / a;
    Vec3SIMD s = r.origin - transformed_v0;
    float u = f * Vec3SIMD::dotfloat(s, h);

    if (u < 0.0 || u > 1.0)
        return false;

    Vec3SIMD q = Vec3SIMD::cross(s, edge1);
    float v = f * Vec3SIMD::dotfloat(r.direction, q);

    if (v < 0.0 || u + v > 1.0)
        return false;

    float t = f * Vec3SIMD::dotfloat(edge2, q);

    if (t < t_min || t > t_max)
        return false;

    rec.t = t;
    rec.point = r.at(t);
    const float w = 1.0 - u - v;

    // Normal hesaplamasý
    rec.interpolated_normal = (w * transformed_n0 + u * transformed_n1 + v * transformed_n2);
    rec.face_normal = Vec3SIMD::cross(edge1, edge2).normalize();

    // Set smoothGroup
    rec.smoothGroup = smoothGroup;

    // Normal hesaplama: Önceki deðeri sakla
    float dot = Vec3SIMD::dotfloat(rec.interpolated_normal, rec.face_normal);
    rec.normal = (dot >= cos_threshold) ? rec.interpolated_normal : rec.face_normal;

    rec.set_face_normal(r, rec.normal);

    // Barycentric koordinatlarý hesapla
    Vec3SIMD bary = calculateBarycentricCoordinates(rec.point);

    // UV koordinatlarýný interpolate et
    Vec2 uv = bary.x() * t0 + bary.y() * t1 + bary.z() * t2;

    rec.uv = uv;
    rec.u = uv.u;
    rec.v = uv.v;
    rec.material = material;

    return true;
}


bool Triangle::bounding_box(double time0, double time1, AABB& output_box) const {
    output_box = AABB(min_point - (EPSILON), max_point + (EPSILON));
    return true;
}

