#include "renderer.h"
#include <SDL_image.h>
#include "SmartUVProjection.h"
#include "SpotLight.h"
#include "AssimpLoader.h"
#include <filesystem>

bool Renderer::isCudaAvailable() {
    try {
        oidn::DeviceRef testDevice = oidn::newDevice(oidn::DeviceType::CUDA);
        testDevice.commit();
        return true; // CUDA destekleniyor
    }
    catch (const std::exception& e) {
        return false; // CUDA desteklenmiyor
    }
}

void Renderer::applyOIDNDenoising(SDL_Surface* surface, int numThreads = 0, bool denoise = true, float blend = 0.8f) {
    Uint32* pixels = static_cast<Uint32*>(surface->pixels);
    int width = surface->w;
    int height = surface->h;

    std::vector<float> colorBuffer(width * height * 3);
    // std::vector<float> normalBuffer(width * height * 3);

    for (int i = 0; i < width * height; ++i) {
        Uint8 r, g, b;
        SDL_GetRGB(pixels[i], surface->format, &r, &g, &b);
        colorBuffer[i * 3] = r / 255.0f;
        colorBuffer[i * 3 + 1] = g / 255.0f;
        colorBuffer[i * 3 + 2] = b / 255.0f;
        // Normal map bilgisini normalBuffer'a ekleyin ve 0-1 aralýðýna normalize edin
      // Önce normal vektörünü normalize et
        //Vec3 normal = Renderer::normalMapBuffer[i].normalize();
        //float x = (normal.x + 1.0f) * 0.5f;
        //float y = (normal.y + 1.0f) * 0.5f;
        //float z = (normal.z + 1.0f) * 0.5f;
        //// Normal verilerini normalBuffer'a aktar
        //normalBuffer[i * 3] = x;
        //normalBuffer[i * 3 + 1] = y;
        //normalBuffer[i * 3 + 2] = z;

    }

    oidn::DeviceRef device;
    if (isCudaAvailable()) {
      //  std::cout << "CUDA destekleniyor, CUDA cihazý kullanýlýyor." << std::endl;
        device = oidn::newDevice(oidn::DeviceType::CUDA);
    }
    else {
        std::cout << "CUDA desteklenmiyor, CPU cihazý kullanýlýyor." << std::endl;
        device = oidn::newDevice(oidn::DeviceType::CPU);
    }
    device.set("numThreads", numThreads);
    device.commit();

    oidn::BufferRef colorOIDNBuffer = device.newBuffer(colorBuffer.size() * sizeof(float));
    //  oidn::BufferRef normalOIDNBuffer = device.newBuffer(normalBuffer.size() * sizeof(float));
    oidn::BufferRef outputOIDNBuffer = device.newBuffer(colorBuffer.size() * sizeof(float));
    std::memcpy(colorOIDNBuffer.getData(), colorBuffer.data(), colorBuffer.size() * sizeof(float));
    // std::memcpy(normalOIDNBuffer.getData(), normalBuffer.data(), normalBuffer.size() * sizeof(float));

    oidn::Quality::High;
   

    oidn::FilterRef filter = device.newFilter("RT");   
    filter.setImage("color", colorOIDNBuffer, oidn::Format::Float3, width, height);
    filter.setImage("output", outputOIDNBuffer, oidn::Format::Float3, width, height);

    filter.set("hdr", false);
    filter.set("srgb", true); // auxiliary images will be prefiltered
    filter.set("denoise", denoise);

    filter.commit();
    // Denoising süresini ölçmek için baþlangýç zamanýný al
    auto start = std::chrono::high_resolution_clock::now();
    // normalfilter.execute();
    filter.execute();

    // Denoising süresini ölçmek için bitiþ zamanýný al
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

   // std::cout << "OIDN Denoiser Uygulandý, süre: " << elapsed.count() << " saniye." << std::endl;

    const char* errorMessage;
    if (device.getError(errorMessage) != oidn::Error::None)
        std::cerr << "OIDN error: " << errorMessage << std::endl;

    std::memcpy(colorBuffer.data(), outputOIDNBuffer.getData(), colorBuffer.size() * sizeof(float));
    for (int i = 0; i < width * height; ++i) {
        Uint8 r_orig, g_orig, b_orig;
        SDL_GetRGB(pixels[i], surface->format, &r_orig, &g_orig, &b_orig);

        float r_denoised = colorBuffer[i * 3];
        float g_denoised = colorBuffer[i * 3 + 1];
        float b_denoised = colorBuffer[i * 3 + 2];

        Uint8 r = static_cast<Uint8>((r_denoised * blend + r_orig / 255.0f * (1 - blend)) * 255);
        Uint8 g = static_cast<Uint8>((g_denoised * blend + g_orig / 255.0f * (1 - blend)) * 255);
        Uint8 b = static_cast<Uint8>((b_denoised * blend + b_orig / 255.0f * (1 - blend)) * 255);

        pixels[i] = SDL_MapRGB(surface->format, r, g, b);
    }
}



Renderer::Renderer( int image_width, int image_height, int samples_per_pixel, int max_depth)
    : image_width(image_width), image_height(image_height),  aspect_ratio(static_cast<double>(image_width) / image_height), halton_cache(MAX_DIMENSIONS, std::vector<float>(MAX_SAMPLES)) 
    {
    initialize_halton_cache();
}

Renderer::~Renderer() {}
void Renderer::set_window(SDL_Window* win) {
    window = win;
}
void Renderer::draw_progress_bar(SDL_Surface* surface, float progress) {
    const int bar_width = surface->w - 40;  // Kenarlarda 20 piksel boþluk býrakýyoruz
    const int bar_height = 20;
    const int bar_y = surface->h - 40;  // Alt kenardan 40 piksel yukarýda

    char percent_text[10];
    snprintf(percent_text, sizeof(percent_text), "%.1f%%", progress * 100);
   
}

void Renderer::render_image(SDL_Surface* surface, SDL_Window* window, const int total_samples_per_pixel, const int samples_per_pass) {
    unsigned int num_threads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    // std::cout << "Starting render with " << num_threads << " threads" << std::endl;
    auto start_time = std::chrono::steady_clock::now();
    Vec3SIMD background_color;

    std::vector<std::shared_ptr<Light>> lights;
    auto [world, bvh] = create_scene(lights, background_color);
    auto create_scene_end_time = std::chrono::steady_clock::now();
    auto create_scene_duration = std::chrono::duration<double, std::milli>(create_scene_end_time - start_time);
    std::cout << "Create Scene Duration: " << create_scene_duration.count() / 1000 << " seconds" << std::endl;
    float blend = 0.2;
    float blendfac = 0;
    if (total_samples_per_pixel > 0)
        blendfac = 0.4 / total_samples_per_pixel;
    std::thread display_thread(&Renderer::update_display, this, window, surface);

    const int num_passes = (total_samples_per_pixel + samples_per_pass - 1) / samples_per_pass;

    for (int pass = 0; pass < num_passes; ++pass) {
        std::cout << "Starting pass " << pass + 1 << " of " << num_passes << std::endl;

        next_row.store(0);  // Reset next_row for each pass

        for (unsigned int t = 0; t < num_threads; ++t) {
            threads.emplace_back(&Renderer::render_worker, this, image_height, surface, std::ref(world),
                std::ref(lights), background_color, bvh.get(), samples_per_pass, pass * samples_per_pass);

        }

        for (auto& thread : threads) {
            thread.join();
        }

        threads.clear();

        // Her geçiþten sonra ilerleme çubuðunu güncelle
        float progress = static_cast<float>(pass + 1) / num_passes;
        // draw_progress_bar(surface, progress);

        std::cout << "\rRendering progress: " << std::fixed << std::setprecision(2) << progress << "%" << std::flush;
        // Pencere baþlýðýný güncelle

        blend = blend + blendfac;

        char title[100];
        snprintf(title, sizeof(title), "Rendering... %.1f%% Complete", progress * 100);
        SDL_SetWindowTitle(window, title);
      
        applyOIDNDenoising(surface, 0, true, blend);
        SDL_UpdateWindowSurface(window);
      
        std::cout << "Pass " << pass + 1 << " completed. Progress: " << (progress * 100) << "%" << std::endl;

    }

    rendering_complete = true;
    display_thread.join();
    applyOIDNDenoising(surface, 0, true, 0.8f);
    SDL_UpdateWindowSurface(window);
    std::cout << "\nRender completed." << std::endl;
    auto render_end_time = std::chrono::steady_clock::now();

    auto render_duration = std::chrono::duration<double, std::milli>(render_end_time - create_scene_end_time);
    auto total_duration = std::chrono::duration<double, std::milli>(render_end_time - start_time);

    std::cout << "Render Duration: " << render_duration.count() / 1000 << " seconds" << std::endl;
    std::cout << "Total Duration: " << total_duration.count() / 1000 << " seconds" << std::endl;

    // Render tamamlandýðýnda pencere baþlýðýný güncelle
    SDL_SetWindowTitle(window, "Render Completed");
}

// Texture yükleme fonksiyonu

struct ObjFile {
    std::string filename;
    std::shared_ptr<Material> material;
    std::string texturePath;  // Objeye ait texture dosya yolu
    
};

std::pair<HittableList, std::shared_ptr<ParallelBVHNode>> Renderer::create_scene(std::vector<std::shared_ptr<Light>>& lights, Vec3SIMD& background_color) {
    
    HittableList world;
    Vec3 v0, v1, v2;
    background_color = { 0.4, 0.5, 0.6 };   
   
    std::vector<std::string> model_files = {
          {"e:/data/home/flower.gltf"},        
      
    // diðer dosya ve malzeme çiftlerini buraya ekleyebilirsiniz
    };

    AssimpLoader assimpLoader;
    baseDirectory = model_files[0];
 
    std::string full_path = model_files[0];
    // Sadece dizin yolunu almak için std::filesystem kullanýyoruz
    std::filesystem::path path(full_path);
    baseDirectory = path.parent_path().string() + "/";
    std::cout << "Dosyanýn dizin yolu: " << baseDirectory << std::endl;

    std::vector<std::pair<Vec3, double>> object_bounds;
    for (const auto& model_file : model_files) {
        std::vector<std::shared_ptr<Triangle>> triangles = assimpLoader.loadModelToTriangles(model_file);

        for (const auto& triangle : triangles) {
           
            world.add(triangle);
        }
    }
   
    auto add_object = [&](std::shared_ptr<Hittable> obj, const Vec3& center, double size) {
        world.add(obj);
        object_bounds.emplace_back(center, size);
        };

    if (lights.empty()) {  // Iþýklar zaten eklenmiþse tekrar ekleme
       // const auto& newLights = assimpLoader.getLights();
      //  lights.insert(lights.end(), newLights.begin(), newLights.end());
        std::cout << "Toplam eklenen ýþýk sayýsý: " << lights.size() << std::endl;

    }

    //lights.push_back(std::make_shared<PointLight>(Vec3(-1.8, 1.68635, 0.5), Vec3(0.3, 0.3, 0.3),0));
    //lights.push_back(std::make_shared<PointLight>(Vec3(-2.2, 1.5, 0.5), Vec3(0.2, 0.2, 0.1), 0));
    lights.push_back(std::make_shared<DirectionalLight>(Vec3(5.0, -2, 2.0), Vec3(0.7, 0.6, 0.3),40.0));    
   //lights.push_back(std::make_shared<AreaLight>(Vec3(2.2, 2.0, 0.8), Vec3(2, -0.5, -0.9), Vec3(2, -1.0, -0.9), 1.0, 1.0, Vec3(1.0, 1.0, 1.0)));
  

   float rx = 85.1262;
   float ry = -66.3824;
   float rz = 0.840447;
   Vec3 forward;
   forward.x = cos(ry) * cos(rz);
   forward.y = sin(rz);
   forward.z = sin(ry) * cos(rz);

   Vec3 lookfrom(-1.8146, 0.290968, -0.381726);
   Vec3 lookat(0, 0.0, 0);
   Vec3 vup(0, 1, 0);
   int blade_count = 4;
   double vfov = 40.0;
   double aperture = 0.0;
   double focus_dist = 30.0;
  
   Vec3 lookfrom1(-2.6, 1.1, 1.8);
   Vec3 lookat1(2.6, 0.5, -1.3);
  
  
   camera = Camera(lookfrom1, lookat1, vup, vfov, aspect_ratio, aperture, focus_dist, blade_count);
    std::cout << "Total objects in the scene: " << world.size() << std::endl;
    // BVH'yi oluþtur       
    auto bvh = std::make_shared<ParallelBVHNode>(world.objects, 0, world.objects.size(), 0.0, 1.0);
    return std::make_pair(world, bvh);
}
std::uniform_int_distribution<> dis_width(0, image_width - 1);
std::uniform_int_distribution<> dis_height(0, image_height - 1);

void Renderer::render_chunk(int start_row, int end_row, SDL_Surface* surface, const HittableList& world,
    const std::vector<std::shared_ptr<Light>>& lights, const Vec3& background_color,
    const ParallelBVHNode* bvh, const int samples_per_pass, const int current_sample) {
   
    // Chunk içindeki pikselleri iþle
    for (int j = end_row; j >= start_row; --j) {
        for (int i = 0; i < image_width; ++i) {
            Vec3 new_color(0, 0, 0);
            // Accumulate colors from multiple samples
            for (int s = 0; s < samples_per_pass; ++s) {
                // Generate ray
                Vec2 uv = stratified_halton(i, j, 0, samples_per_pass);
                Ray r = camera.get_ray(uv.u, uv.v);
                // Calculate ray color
                new_color += ray_color(r, bvh, lights, background_color, MAX_DEPTH, 0);
            }

            // Mevcut rengi al (lineer uzayda)
            std::lock_guard<std::mutex> lock(mtx);
            Uint32* pixel = static_cast<Uint32*>(surface->pixels) + (image_height - 1 - j) * surface->pitch / 4 + i;
            Uint8 r, g, b;
            SDL_GetRGB(*pixel, surface->format, &r, &g, &b);
            Vec3 existing_color(r / 255.0, g / 255.0, b / 255.0);
            existing_color = Vec3(existing_color.x * existing_color.x, existing_color.y * existing_color.y, existing_color.z * existing_color.z); // Gamma decode

            // Mevcut ve yeni renkleri lineer uzayda birleþtir
            int total_samples = current_sample + samples_per_pass;
            Vec3 combined_color = (existing_color * current_sample + new_color) / total_samples;

            // Gamma düzeltmesi
            combined_color = Vec3(sqrt(combined_color.x), sqrt(combined_color.y), sqrt(combined_color.z));

            // Rengi SDL piksel formatýna dönüþtür
            int ir = static_cast<int>(256 * clamp(combined_color.x, 0.0, 0.999));
            int ig = static_cast<int>(256 * clamp(combined_color.y, 0.0, 0.999));
            int ib = static_cast<int>(256 * clamp(combined_color.z, 0.0, 0.999));

            // SDL yüzeyinde piksel rengini ayarla
            *pixel = SDL_MapRGB(surface->format, ir, ig, ib);
        }
    }
}

void Renderer::render_worker(int image_height, SDL_Surface* surface, const HittableList& world,
    const std::vector<std::shared_ptr<Light>>& lights, const Vec3& background_color,
    const ParallelBVHNode* bvh, const int samples_per_pass, const int current_sample) {

    const int chunk_size = 16;

    while (true) {
        int start_row = next_row.fetch_add(chunk_size, std::memory_order_relaxed);
        if (start_row >= image_height) {
            break;
        }
        int end_row = std::min(start_row + chunk_size - 1, image_height - 1);
        render_chunk(start_row, end_row, surface, world, lights, background_color, bvh, samples_per_pass, current_sample);
    }
}


void Renderer::update_display(SDL_Window* window, SDL_Surface* surface) {
  
    while (!rendering_complete) {
      
        SDL_UpdateWindowSurface(window);      
        float progress = static_cast<float>(completed_pixels) / (image_width * image_height) * 100.0f;
      
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                rendering_complete = true;
                return;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 100ms aralýklarla güncelle
    }
   
}


std::vector<Vec3SIMD> Renderer::normalMapBuffer;
std::mutex Renderer::normalMapBufferMutex;


// Yeni metod: Normal map uygulama
Vec3SIMD Renderer::apply_normal_map(const HitRecord& rec) {

    if (rec.material->has_normal_map()) {
        Vec3SIMD tangent, bitangent;
        create_coordinate_system(rec.normal.normalize(), tangent, bitangent);
        Vec3SIMD normal_from_map = rec.material->get_normal_from_map(rec.u, rec.v);
        normal_from_map = normal_from_map * 1.75 - Vec3(1, 1, 1);
        normal_from_map *= rec.material->get_normal_strength();
        Vec3SIMD transformed_normal(
            tangent.x() * normal_from_map.x() + bitangent.x() * normal_from_map.y() + rec.normal.x() * normal_from_map.z(),
            tangent.y() * normal_from_map.x() + bitangent.y() * normal_from_map.y() + rec.normal.y() * normal_from_map.z(),
            tangent.z() * normal_from_map.x() + bitangent.z() * normal_from_map.y() + rec.normal.z() * normal_from_map.z()
        );

        std::lock_guard<std::mutex> lock(normalMapBufferMutex);

        normalMapBuffer.push_back(normal_from_map.normalize());

         normalMapBuffer.push_back(rec.material->get_normal_from_map(rec.u, rec.v).normalize()); 
            return transformed_normal.normalize();
    }
    return Vec3SIMD(rec.normal.normalize());
}

// Yeni metod: Koordinat sistemi oluþturma
void Renderer::create_coordinate_system(const Vec3SIMD& N, Vec3SIMD& T, Vec3SIMD& B) {
    if (std::fabs(N.x()) > std::fabs(N.y())) {
        T = Vec3SIMD(N.z(), 0, -N.x()).normalize();
    }
    else {
        T = Vec3SIMD(0, -N.z(), N.y()).normalize();
    }
    B = Vec3SIMD::cross(N, T);
}
void Renderer::initialize_halton_cache() {
    for (int d = 0; d < MAX_DIMENSIONS; ++d) {
        int base = (d == 0) ? 2 : 3; // Ýlk boyut için 2 tabaný, ikinci boyut için 3 tabaný kullan
        for (int i = 0; i < MAX_SAMPLES; ++i) {
            halton_cache[d][i] = halton(i, base);
        }
    }
}
void Renderer::precompute_halton(int max_index) {
    for (int i = 0; i < 2; ++i) {
        halton_cache[i].resize(max_index);
        for (int j = 0; j < max_index; ++j) {
            halton_cache[i][j] = halton(j, i == 0 ? 2 : 3);
        }
    }
}
float Renderer::get_halton_value(int index, int dimension) {
    if (index < max_halton_index) {
        return halton_cache[dimension][index];
    }
    else {
        return halton(index, dimension == 0 ? 2 : 3);
    }
}
// Halton dizisi kullanma örneði
float Renderer::halton(int index, int base) {
    float f = 1, r = 0;
    while (index > 0) {
        f = f / base;
        r = r + f * (index % base);
        index = index / base;
    }
    return r;
}

Vec2 Renderer::stratified_halton(int x, int y, int sample_index, int samples_per_pixel) {
    int index = (y * image_width + x) * samples_per_pixel + sample_index;
    index = index % MAX_SAMPLES;

    float u = halton_cache[0][index];
    float v = halton_cache[1][index];

    // Piksel içinde rastgele offset ekleyelim
    float offset_u = static_cast<float>(rand()) / RAND_MAX;
    float offset_v = static_cast<float>(rand()) / RAND_MAX;

    u = (u + offset_u) / samples_per_pixel;
    v = (v + offset_v) / samples_per_pixel;

    return Vec2((x + u) / image_width, (y + v) / image_height);
}

Vec3SIMD Renderer::ray_color(const Ray& r, const ParallelBVHNode* bvh, const std::vector<std::shared_ptr<Light>>& lights, const Vec3SIMD& background_color, int depth, int sample_index) {
    Vec3SIMD final_color(0, 0, 0);
    Vec3SIMD throughput(1, 1, 1);
    Ray current_ray = r;
    float total_distance = 0.0f;
    Vec3SIMD sky_color;   
    const int MAX_DEPTH_DIELECTRIC = 15;
    const int MAX_DEPTH_VOLUMETRIC = 20;
    const int MAX_DEPTH_METAL = 6;
    const int MAX_DEPTH_PrincipledBSDF = 6;
    const int MIN_DEPTH = 3;
    const int MAX_DEPTH = std::max({ MAX_DEPTH_DIELECTRIC, MAX_DEPTH_VOLUMETRIC, MAX_DEPTH_METAL, MAX_DEPTH_PrincipledBSDF });
    const float BASE_THRESHOLD = 0.01f;
    const float DISTANCE_FACTOR = 0.001f;

    //static int global_sample_index = 0;
    static std::atomic<int> global_sample_index(0);
    int local_sample_index = global_sample_index.fetch_add(1, std::memory_order_relaxed);

    for (int bounce = 0; bounce < MAX_DEPTH; ++bounce) {
        // Dinamik erken çýkýþ stratejisi
        float dynamic_threshold = BASE_THRESHOLD + (total_distance * DISTANCE_FACTOR);
        if (bounce > MIN_DEPTH) {
            float max_component = throughput.max_component();
            float continuation_probability = std::min(max_component, 0.95f);

            if (get_halton_value(local_sample_index, 0) > continuation_probability || max_component < dynamic_threshold) {
                break;
            }

            throughput /= continuation_probability;
        }
        HitRecord rec;
        if (!bvh->hit(current_ray, EPSILON, std::numeric_limits<double>::infinity(), rec)) {
            if (atmosphericEffects.enable) {
                if (background_texture) {
                    float u = 0.5f + std::atan2(current_ray.direction.z(), current_ray.direction.x()) / (2 * M_PI);
                    float v = 0.5f - std::asin(current_ray.direction.y()) / M_PI;
                    sky_color = background_texture->get_color(u, v);
                }
                else {
                    sky_color = background_color;
                }
                final_color += throughput * atmosphericEffects.applyAtmosphericEffects(sky_color, total_distance);
            }
            else {
                final_color += throughput * background_color;
            }
            break;
        }

        Vec3SIMD original_normal(rec.normal);
        Vec3SIMD transformed_normal = apply_normal_map(rec);
        rec.normal = static_cast<Vec3SIMD>(transformed_normal);

        float segment_distance = rec.t;
        total_distance += segment_distance;

        if (atmosphericEffects.enable) {
            throughput = atmosphericEffects.attenuateSegment(throughput, total_distance - segment_distance, total_distance);
            Vec3SIMD segment_contribution = atmosphericEffects.calculateSegmentContribution(total_distance - segment_distance, total_distance);
            final_color += throughput * segment_contribution;
        }

        int max_depth_for_material;
        switch (rec.material->type()) {
        case MaterialType::Dielectric:
            max_depth_for_material = MAX_DEPTH_DIELECTRIC;
            break;
        case MaterialType::Volumetric:
            max_depth_for_material = MAX_DEPTH_VOLUMETRIC;
            break;
        case MaterialType::Metal:
            max_depth_for_material = MAX_DEPTH_METAL;
            break;
        case MaterialType::PrincipledBSDF:
        default:
            max_depth_for_material = MAX_DEPTH_PrincipledBSDF;
            break;
        }

        if (bounce >= max_depth_for_material && bounce >= MIN_DEPTH) {
            break;
        }
       

        Vec3SIMD emitted = Vec3SIMD(rec.material->emitted(rec.u, rec.v, rec.point));
        final_color += throughput * emitted;
        if (rec.material->type() == MaterialType::Dielectric) {
            Vec3SIMD attenuation;
            Ray scattered;
            bool is_scattered = rec.material->scatter(current_ray, rec, attenuation, scattered);

            if (!is_scattered) break;

            throughput *= Vec3SIMD(attenuation);
            current_ray = scattered;

        }

      
       
        else {
            Vec3SIMD attenuation;
            Ray scattered;
            if (!rec.material->scatter(current_ray, rec, attenuation, scattered)) {
                break;
            }

            if (rec.material->type() != MaterialType::Dielectric && rec.material->type() != MaterialType::Volumetric) {
                Vec3SIMD direct_light = calculate_direct_lighting(bvh, lights, rec, rec.normal);
                final_color += throughput * Vec3SIMD(attenuation) * direct_light;
            }

            rec.normal = static_cast<Vec3>(original_normal);

            // Saçýlan ýþýk zayýflamasý
            float scattering_attenuation = 1.0f / (1.0f + total_distance *  rec.material->get_scattering_factor()); // Ayarlanabilir faktör
           
            throughput *= Vec3SIMD(attenuation) * scattering_attenuation;
           
            // Rus ruleti
            if (bounce > MIN_DEPTH) {
                float p;
                switch (rec.material->type()) {
                case MaterialType::Dielectric:
                case MaterialType::Volumetric:
                    p = std::max(0.8f, std::min(0.99f, throughput.max_component()));
                    break;
                case MaterialType::Metal:
                    p = std::max(0.5f, std::min(0.97f, throughput.max_component()));
                    break;
                case MaterialType::PrincipledBSDF:
                default:
                    p = std::max(0.3f, std::min(0.95f, throughput.max_component()));
                    break;
                }

                if (get_halton_value(local_sample_index + bounce, 1) >= p) {
                    break;
                }
                throughput /= p;
            }

            current_ray = scattered;
        }
    }

    return final_color;
}

Vec3SIMD Renderer::calculate_light_contribution(
    const std::shared_ptr<Light>& light,
    const Vec3SIMD& point,
    const Vec3SIMD& geometric_normal,
    const Vec3SIMD& shading_normal,
    const Vec3SIMD& view_direction,
    float shininess,
    float metallic,
    bool is_global
) {
    Vec3SIMD to_light;
    Vec3SIMD intensity = light->intensity;
    float distance = 0.0f;
    bool is_directional = false;

    if (auto directional_light = std::dynamic_pointer_cast<DirectionalLight>(light)) {
      
        distance = 1000.0f; // Assume a far distance for directional lights
        is_directional = true;     
        Vec3SIMD point_vec3 = point;
       
        to_light = - directional_light->random_point().normalize(); 
            
    }
    else if (auto point_light = std::dynamic_pointer_cast<PointLight>(light)) {
        Vec3SIMD light_pos = point_light->getPosition();
        Vec3SIMD point_vec3 = point;
        to_light = light_pos - point_vec3;
        distance = to_light.length();
        to_light = to_light.normalize();
        is_directional = false;
        intensity = point_light->getIntensity() / (distance * distance);
    }
    else if (auto spot_light = std::dynamic_pointer_cast<SpotLight>(light)) {
        Vec3SIMD light_pos = spot_light->position;
        Vec3SIMD point_vec3 = point;
        to_light = light_pos - point_vec3;
        distance = to_light.length();
        to_light = to_light.normalize();
        is_directional = false;

        // Spot ýþýðýn yönü ile noktaya olan yön arasýndaki açýyý kontrol et
        float cos_theta = Vec3::dot(to_light, spot_light->direction);
        if (cos_theta > std::cos(spot_light->angle_degrees)) {
            // Eðer nokta spot ýþýðýn konisi içindeyse, ýþýk þiddetini hesapla
            float falloff = std::pow(cos_theta, 2.0f);  // Cosine falloff; exponent ayarlanabilir
            intensity = spot_light->intensity * falloff / (distance * distance);
        }
        else {
            // Nokta spot ýþýðýn konisi dýþýnda, bu yüzden ýþýk þiddeti sýfýr
            intensity = Vec3SIMD(0.0, 0.0, 0.0);
        }
    }

    else if (auto area_light = std::dynamic_pointer_cast<AreaLight>(light)) {
        Vec3SIMD random_light_pos = area_light->random_point();
        Vec3SIMD point_vec3 = point;
        to_light = random_light_pos - point_vec3;
        distance = to_light.length();
        to_light = to_light.normalize();
        is_directional = false;
        double area = area_light->getWidth() * area_light->getHeight();
        double intensity_factor = area_light->getIntensity().length() / area;
        double attenuation = 1.0 / (distance * distance);
        intensity = area_light->getIntensity() * attenuation;
    }

    // Light atmospheric effect
    float fog_density = 0.00f;
    float haze_density = 0.0f;
    float atmospheric_attenuation = expf(-distance * (fog_density + haze_density));
    intensity = intensity * atmospheric_attenuation;

    __m256 cos_theta_vec = Vec3SIMD::dot(shading_normal, to_light).data;
    // cos(theta) deðerini 0 ile 1 arasýnda sýnýrlýyoruz
    cos_theta_vec = _mm256_max_ps(_mm256_set1_ps(0.0f), cos_theta_vec);  // 0'dan küçük deðerleri 0 yap
    cos_theta_vec = _mm256_min_ps(_mm256_set1_ps(1.0f), cos_theta_vec);  // 1'den büyük deðerleri 1 yap

    // Eðer global illumination aktifse, ýþýk yoðunluðunu artýr
    __m256 intensity_vec = intensity.data;
    if (is_global) {
        intensity_vec = _mm256_mul_ps(intensity_vec, _mm256_set1_ps(1.3f));  // intensity * 1.3
    }

    __m256 result_vec = _mm256_mul_ps(intensity_vec, cos_theta_vec);

    // Sonucu Vec3SIMD objesine dönüþtürüp döndür
    Vec3SIMD result;
    result.data = result_vec;
    return result;
}

Vec3SIMD Renderer::calculate_direct_lighting(
    const ParallelBVHNode* bvh,
    const std::vector<std::shared_ptr<Light>>& lights,
    const HitRecord& rec,
    const Vec3SIMD& normal
) {
    const Vec3SIMD& background_color = Vec3(0, 0, 0);
    Vec3SIMD direct_light(0, 0, 0);
    const Vec3SIMD& hit_point = rec.point;
    const Vec3SIMD& hit_normal = normal;  // Use the provided normal instead of rec.normal
    Vec3SIMD shading_normal = apply_normal_map(rec);
    HitRecord shadow_rec;  // Create a single HitRecord object and reuse it
    Vec3SIMD view_direction = (camera_position - hit_point).normalize();
    // Sadece rastgele seçilmiþ bir ýþýk kaynaðýný örnekle
    ThreadLocalRNG rng;
    int light_index = static_cast<int>(rng.get() * lights.size());
    const auto& light = lights[light_index];
    float shininess = rec.material->get_shininess();
    float metallic = rec.material->get_metallic();
    Vec3SIMD to_light;
    float light_distance = std::numeric_limits<float>::infinity();
    Vec3SIMD light_contribution;
    Ray shadow_ray(hit_point, to_light);
    float transmittance = 1.0f;
    bool blocked = false;

   

    if (!blocked) {
        direct_light += light_contribution * transmittance;
    }
    for (const auto& light : lights) {
       
        if (const auto* directional_light = dynamic_cast<const DirectionalLight*>(light.get())) {
            //to_light = -directional_light->direction.normalize();
           
          
            to_light =  - directional_light->random_point().normalize();
            light_contribution = calculate_light_contribution(light, hit_point, hit_normal, shading_normal, view_direction, shininess, metallic);
        }
        else if (const auto* point_light = dynamic_cast<const PointLight*>(light.get())) {
            to_light = Vec3SIMD(point_light->getPosition()) - hit_point;
            light_distance = to_light.length();
            to_light = to_light.normalize();
            light_contribution = calculate_light_contribution(light, hit_point, hit_normal, shading_normal, view_direction, shininess, metallic);
        }
        else if (const auto* spot_light = dynamic_cast<const SpotLight*>(light.get())) {
            to_light = Vec3SIMD(spot_light->position) - hit_point;
            light_distance = to_light.length();
            to_light = to_light.normalize();

            // Spot ýþýðýn yönü ile noktanýn konumu arasýndaki açýyý kontrol et
            float cos_theta = to_light.dot(spot_light->direction);
            if (cos_theta > std::cos(spot_light->angle_degrees)) {
                // Spot ýþýk konisinin içindeyse katkýyý hesapla
                float falloff = std::pow(cos_theta, 2.0f); // Cosine falloff; exponent ayarlanabilir
                light_contribution = calculate_light_contribution(light, hit_point, hit_normal, shading_normal, view_direction, shininess, metallic) * falloff;
            }
            else {
                // Nokta koni dýþýnda, katký sýfýr
                light_contribution = Vec3SIMD(0.0f, 0.0f, 0.0f);
            }
        }
        else if (const auto* area_light = dynamic_cast<const AreaLight*>(light.get())) {
            constexpr int num_samples = 16;
            const float area = area_light->getWidth() * area_light->getHeight();
            const float intensity_factor = area_light->getIntensity().length() / area;
            Vec3SIMD area_light_contribution(0, 0, 0);

            for (int i = 0; i < num_samples; ++i) {
                Vec3SIMD sample_point = area_light->random_point();
                to_light = sample_point - hit_point;
                light_distance = to_light.length();
                to_light = to_light.normalize();
                area_light_contribution += calculate_light_contribution(light, hit_point, hit_normal, shading_normal, view_direction, shininess, metallic);
            }

            light_contribution = area_light_contribution / static_cast<float>(num_samples);
        }
        else {
            continue;
        }

        if (!bvh->hit(Ray(hit_point, to_light), 0.01f, light_distance, shadow_rec)) {

            direct_light += light_contribution;
        }
    }
   
    return direct_light;
}
