/* 1.07.20024
* 
  Ray Tracing Project

  This file contains an implementation of a ray tracing application written in C++,
  designed to render 3D scenes realistically using ray tracing techniques. It includes
  optimizations using SIMD and multi-threading for performance.

  Created by Kemal DEMÝRTAÞ and licensed under the MIT License.
*/


/*
  Ray Tracing Projesi

  Bu dosya, ray tracing tekniðini kullanarak 3D sahneleri foto-gerçekçi bir þekilde render etmek için
  C++ dilinde yazýlmýþ bir uygulamayý içerir. SIMD kullanýmý ve çoklu iþ parçacýðý desteði ile performans
  optimizasyonu saðlanmýþtýr.

  Proje, Kemal DEMÝRTAÞ tarafýndan oluþturulmuþtur ve MIT Lisansý altýnda lisanslanmýþtýr.
*/




#ifndef RENDERER_H
#define RENDERER_H

#include <SDL.h>
#include <vector>
#include <memory>
#include <thread>
#include <chrono>
#include <iostream>
#include <iomanip>
#include "HittableList.h"
#include "light.h"
#include "Vec3.h"
#include "Camera.h"
#include "globals.h"
#include "PointLight.h"
#include "DirectionalLight.h"
#include "AreaLight.h"
#include "Volumetric.h"
#include "matrix4x4.h"
#include "DirectionalLight.h"
#include "PrincipledBSDF.h"
#include "Dielectric.h"
#include "Metal.h"
#include "Material.h"
#include "Triangle.h"
#include "Vec2.h"
#include "Vec3SIMD.h"
#include "Mesh.h"
#include "AABB.h"
#include "Ray.h"
#include "Hittable.h"
#include "EmissiveMaterial.h"
#include "DiffuseLight.h"
#include "ThreadLocalRNG.h"
#include "AtmosphericEffects.h"
#include "ParallelBVHNode.h"
#include <OpenImageDenoise/oidn.hpp>


class Renderer {
public:

    static bool isCudaAvailable();

    static void applyOIDNDenoising(SDL_Surface* surface, int numThreads, bool denoise, float blend);

    Renderer(int image_width, int image_height, int max_depth, int samples_per_pixel);;
    ~Renderer();
    int max_halton_index = 1000; // Örnek bir deðer, ihtiyaca göre ayarlayýn

  
    void precompute_halton(int max_halton_index);
    float get_halton_value(int index, int dimension);
    static float halton(int index, int base);
    Vec2 stratified_halton(int pixel_x, int pixel_y, int sample_index, int samples_per_pixel);
    static void create_coordinate_system(const Vec3SIMD& N, Vec3SIMD& T, Vec3SIMD& B);
    void initialize_halton_cache();
    static std::vector<Vec3SIMD> normalMapBuffer;
    static std::mutex normalMapBufferMutex;
     
    //std::pair<HittableList, std::shared_ptr<BVHNode>> create_scene(std::vector<std::shared_ptr<Light>>& lights, Vec3& background_color);
     std::pair<HittableList, std::shared_ptr<ParallelBVHNode>> create_scene(std::vector<std::shared_ptr<Light>>& lights, Vec3SIMD& background_color);
     Vec3 adaptive_sample_pixel(int i, int j, const Camera& cam, const ParallelBVHNode* bvh, const std::vector<std::shared_ptr<Light>>& lights, const Vec3& background_color, int max_samples_per_pixel, double variance_threshold, int variance_check_interval);
     void render_chunk(int start_row, int end_row, SDL_Surface* surface, const HittableList& world, const std::vector<std::shared_ptr<Light>>& lights, const Vec3& background_color, const ParallelBVHNode* bvh, const int samples_per_pass, const int current_sample);

     void update_pixel_color(SDL_Surface* surface, int i, int j, const Vec3SIMD& color, int current_sample, int new_samples);
  
    void set_window(SDL_Window* win);

    void draw_progress_bar(SDL_Surface* surface, float progress);

    void render_image(SDL_Surface* surface, SDL_Window* window, const int total_samples_per_pixel, const int samples_per_pass);
    void set_camera_position(const Vec3SIMD& position) {
        camera_position = position;
    }
    void initializeBuffers(int image_width, int image_height);
    static Vec3SIMD apply_normal_map(const HitRecord& rec);
private:
  
    Camera camera;
    int image_width;
    int image_height;
    double aspect_ratio;
    int samples_per_pixel;
    int max_depth;
    std::vector<std::vector<float>> halton_cache;
    std::atomic<int> next_pixel{ 0 };
    const int total_pixels= image_width* image_height;
    const int chunk_size = 64; // Önbellek dostu chunk boyutu
    static const int MAX_DIMENSIONS = 5; // Halton dizisi için maksimum boyut
    static const int MAX_SAMPLES = 1024; // Maksimum örnek sayýsý
    std::atomic<int> next_row{ 0 };
    std::atomic<bool> rendering_complete{ false };
    // Rastgele sýralama (shuffle)
    std::random_device rd;

   
    std::mutex mtx;
    float max(float a, float b) const { return a > b ? a : b; }

    Vec3SIMD camera_position;
    AtmosphericEffects atmosphericEffects;
    SDL_Renderer* sdlRenderer; // SDL_Renderer pointer'ý ekleyin
    std::shared_ptr<Texture> background_texture;
    Vec3SIMD sample_directional_light(const ParallelBVHNode* bvh, const DirectionalLight* light, const HitRecord& rec, const Vec3SIMD& light_contribution);
    Vec3SIMD sample_point_light(const ParallelBVHNode* bvh, const PointLight* light, const HitRecord& rec, const Vec3SIMD& light_contribution);
    Vec3SIMD sample_area_light(const ParallelBVHNode* bvh, const AreaLight* light, const HitRecord& rec, const Vec3SIMD& light_contribution, int num_samples);
    
    void render_worker(int image_height, SDL_Surface* surface, const HittableList& world, const std::vector<std::shared_ptr<Light>>& lights, const Vec3& background_color, const ParallelBVHNode* bvh, const int samples_per_pass, const int current_sample);
    
    void update_display(SDL_Window* window, SDL_Surface* surface);
   
  
 
    Vec3SIMD ray_color(const Ray& r, const ParallelBVHNode* bvh, const std::vector<std::shared_ptr<Light>>& lights, const Vec3SIMD& background_color, int depth=0, int sample_index=0);
    Vec3SIMD calculate_volumetric_lighting(const ParallelBVHNode* bvh, const std::vector<std::shared_ptr<Light>>& lights, const HitRecord& rec, const Ray& ray);
    Vec3SIMD calculate_light_contribution(const std::shared_ptr<Light>& light, const Vec3SIMD& point, const Vec3SIMD& geometric_normal, const Vec3SIMD& shading_normal, const Vec3SIMD& view_direction, float shininess, float metallic, bool is_global=true);
    Vec3SIMD calculate_direct_lighting(const ParallelBVHNode* bvh, const std::vector<std::shared_ptr<Light>>& lights, const HitRecord& rec, const Vec3SIMD& normal);
  
    Vec3 apply_atmospheric_effects(const Vec3& intensity, float distance, bool is_global);
    Vec3SIMD calculate_specular(const Vec3& intensity, const Vec3SIMD& normal, const Vec3SIMD& to_light, const Vec3SIMD& view_direction, float shininess);
 
    Vec3SIMD calculate_diffuse(const Vec3& intensity, float cos_theta, float metallic);
    Vec3SIMD calculate_global_illumination(const ParallelBVHNode* bvh, const std::vector<std::shared_ptr<Light>>& lights, const HitRecord& rec, const Vec3SIMD& normal, const Vec3SIMD& view_direction, const Vec3SIMD& background_color);
   
    
      SDL_Window* window;
  
    
};

#endif // RENDERER_H