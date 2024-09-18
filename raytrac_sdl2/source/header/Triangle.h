#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <memory>
#include <string>
#include <algorithm>
#include "Hittable.h"
#include "Material.h"
#include "Vec2.h"
#include "Matrix4x4.h"
#include "Vec3SIMD.h"
#include <SDL.h>
#include <SDL_image.h>


class Mesh;

class Triangle : public Hittable {
public:

    std::shared_ptr<Mesh> mesh; // Mesh nesnesine referans
    Vec3SIMD v0, v1, v2;  // Vertices
    Vec3SIMD n0, n1, n2;  // Normals
    Vec2 t0, t1, t2;  // Texture coordinates
   
    std::shared_ptr<Material> material;
    Matrix4x4 transform;
    Vec3SIMD albedo0, albedo1, albedo2;
    float metallic0, metallic1, metallic2;
    float roughness0, roughness1, roughness2;
    void initialize_transforms() {
        // Transform iþlemlerini sadece bir kez yapýp sonuçlarý sakla
        transformed_v0 = transform.transform_point(v0);
        transformed_v1 = transform.transform_point(v1);
        transformed_v2 = transform.transform_point(v2);

        transformed_n0 = transform.transform_vector(n0).normalize();
        transformed_n1 = transform.transform_vector(n1).normalize();
        transformed_n2 = transform.transform_vector(n2).normalize();
    }

    std::shared_ptr<Texture> texture;
    std::string materialName;
    int smoothGroup;
    // Dönüþtürülmüþ haller
    Vec3SIMD transformed_v0, transformed_v1, transformed_v2;
    Vec3SIMD transformed_n0, transformed_n1, transformed_n2;

    // Default constructor
    Triangle();

    // Constructor with vertices and material
    Triangle(const Vec3SIMD& a, const Vec3SIMD& b, const Vec3SIMD& c, std::shared_ptr<Material> m);

    Triangle(const Vec3SIMD& a, const Vec3SIMD& b, const Vec3SIMD& c,
        const Vec3SIMD& na, const Vec3SIMD& nb, const Vec3SIMD& nc,
        const Vec2& ta, const Vec2& tb, const Vec2& tc,
        std::shared_ptr<Material> m, int sg);

    void setUVCoordinates(const Vec2& uv0, const Vec2& uv1, const Vec2& uv2);
        
    // Set material
    void setMaterial(std::shared_ptr<Material> m) { material = m; }
    // Set transformation matrix
    void set_transform(const Matrix4x4& t);
    static void updateTriangleTransform(Triangle& triangle, const Matrix4x4& transform);
    void render(SDL_Renderer* renderer, SDL_Texture* texture);
    std::tuple<Vec2, Vec2, Vec2> getUVCoordinates() const;
    // Set normals
    void set_normals(const Vec3SIMD& normal0, const Vec3SIMD& normal1, const Vec3SIMD& normal2);

    // Override hit function for ray-triangle intersection
    virtual bool hit(const Ray& r, double t_min, double t_max, HitRecord& rec) const override;

    // Override bounding box function for bounding volume hierarchy (BVH)
    virtual bool bounding_box(double time0, double time1, AABB& output_box) const override;
   
private:
    Vec3SIMD calculateBarycentricCoordinates(const Vec3SIMD& point) const;

    void applyUVCoordinatesToHitRecord(HitRecord& hitRecord, const std::shared_ptr<Triangle>& triangle);
    
    Vec3SIMD min_point;
    Vec3SIMD max_point;
    void update_bounding_box();
    Vec2 uv0, uv1, uv2;
};

#endif // TRIANGLE_H
