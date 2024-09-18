#pragma once
#include "Vec3SIMD.h"
#include "Texture.h"
#include <string>
#include <optional>

class AtmosphericEffects {
private:
    float fog_start_distance;
    float fog_base_density;
    float fog_distance_factor;
    float haze_density;
    Vec3 fog_color;
    Vec3 haze_color;
    std::optional<Texture> fog_texture;
    std::optional<Texture> haze_texture;
    std::optional<Texture> background_texture;
    Vec3 background_color;
    //bool use_background_texture;

public:
    bool use_background_texture;
    bool enable = true;
    Vec3SIMD attenuateSegment(const Vec3SIMD& color, float start_distance, float end_distance) const;
    Vec3SIMD calculateSegmentContribution(float start_distance, float end_distance) const;

    AtmosphericEffects(
        float fog_start = 0.0f,
        float fog_base = 0.0f,
        float fog_factor = 0.0f,
        float haze = 0.0f,
        const Vec3SIMD& fog_col = Vec3SIMD(0.5f, 0.5f, 0.5f),
        const Vec3SIMD& haze_col = Vec3SIMD(0.7f, 0.8f, 1.0f),
        const Vec3SIMD& bg_col = Vec3SIMD(0.0f, 0.0f, 0.0f));

    void setFogTexture(const std::string& texture_path);
    void setHazeTexture(const std::string& texture_path);
    void setBackgroundTexture(const std::string& texture_path);
    void setBackgroundColor(const Vec3SIMD& color);
    Vec3SIMD getBackgroundColor(float u = 0.0f, float v = 0.0f) const;

    void setFogStartDistance(float distance);
    void setFogBaseDensity(float density);
    void setFogDistanceFactor(float factor);
    void setHazeDensity(float density);
    void setFogColor(const Vec3SIMD& color);
    void setHazeColor(const Vec3SIMD& color);

    float getFogStartDistance() const;
    float getFogBaseDensity() const;
    float getFogDistanceFactor() const;
    float getHazeDensity() const;
    Vec3 getFogColor() const;
    Vec3 getHazeColor() const;

    float calculateFogFactor(float distance) const;
    float calculateHazeFactor(float distance) const;
    Vec3SIMD applyAtmosphericEffects(const Vec3SIMD& color, float distance, float u = 0.0f, float v = 0.0f) const;
};