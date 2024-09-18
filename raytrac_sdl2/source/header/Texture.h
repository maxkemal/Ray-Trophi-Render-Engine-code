#pragma once
#include <vector>
#include <string>
#include "Vec3.h"
#include <SDL_image.h>
#include "Vec2.h"

class Texture {
private:
    bool is_gray_scale = true; // Siyah-beyaz (gri tonlama) kontrolü için bir bayrak
    std::vector<Vec3> pixels;
    int width;
    int height;
     bool m_is_loaded = false;
     std::vector<float> alphas;    // Alfa kanalý verileri için
public:
    Texture(const std::string& filename);
    Vec3 getColor(const Vec2& uv) const {
        return get_color(uv.u, uv.v);
    }
    void loadOpacityMap(const std::string& filename);
    float get_alpha(double u, double v) const; // Yeni metod
    Vec3 get_color(double u, double v) const;
    ~Texture();
    bool is_loaded() const { return m_is_loaded; }
    
};
