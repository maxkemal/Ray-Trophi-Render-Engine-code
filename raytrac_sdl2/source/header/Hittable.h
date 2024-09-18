#ifndef HITTABLE_H
#define HITTABLE_H

#include "Ray.h"
#include "AABB.h"
#include <vector>
#include <memory> // std::shared_ptr kullan�m� i�in
#include "Vec2.h"

class Material; // �leri bildirim
class Texture;

struct HitRecord {
    Vec3SIMD point;
    Vec3SIMD normal;
    Vec3SIMD neighbor_normal;
    bool has_neighbor_normal = false;
    Vec3SIMD interpolated_normal;
    Vec3SIMD face_normal;
    int smoothGroup;
    std::shared_ptr<Material>  material;
    double t;
    double u;
    double v;
    bool front_face;
    // Yeni alanlar
 
    Vec2 uv; // UV koordinatlar� eklendi
    
    Vec2 globalUV; // T�m obje i�in global UV koordinatlar�
    bool hasGlobalUV = false; // Global UV'nin set edilip edilmedi�ini kontrol etmek i�in

    inline void set_face_normal(const Ray& r, const Vec3& outward_normal) {
        front_face = Vec3::dot(r.direction, outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

class Hittable {
public:
    virtual bool hit(const Ray& r, double t_min, double t_max, HitRecord& rec) const = 0;

    virtual bool bounding_box(double time0, double time1, AABB& output_box) const = 0;
    virtual ~Hittable() = default;
    virtual void collect_neighbor_normals(const AABB& query_box, Vec3SIMD& neighbor_normal,
        int& neighbor_count, const std::shared_ptr<Material>& current_material) const {
        // Varsay�lan implementasyon: hi�bir �ey yapma
    }
    std::vector<std::shared_ptr<Hittable>> objects; // objects �yesi eklendi
};

#endif // HITTABLE_H
