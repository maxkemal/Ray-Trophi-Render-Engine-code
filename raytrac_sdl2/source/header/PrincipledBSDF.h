#pragma once

#include "Material.h"
#include "Texture.h"
#include "Vec2.h"
#include "Vec3.h"
#include "Vec3SIMD.h"
#include "Ray.h"
#include "Hittable.h"
#include <memory>
#include "SmartUVProjection.h"



class PrincipledBSDF : public Material {
public:
    struct TextureTransform {
        Vec2 scale;
        double rotation_degrees;
        Vec2 translation;
        Vec2 tilingFactor;
        WrapMode wrapMode;

        TextureTransform(
            const Vec2& scale = Vec2(1.0, 1.0),
            double rotation = 0.0,
            const Vec2& translation = Vec2(0.0, 0.0),
            const Vec2& tiling = Vec2(1.0, 1.0),
            WrapMode wrap = WrapMode::Repeat
        ) : scale(scale), rotation_degrees(rotation), translation(translation),
            tilingFactor(tiling), wrapMode(wrap) {}
    };

  
   // PrincipledBSDF(std::shared_ptr<Texture> tex) : texture(tex) {}
    // Existing constructors
    PrincipledBSDF();
    PrincipledBSDF(const Vec3& albedo, float roughness, float metallic);
    PrincipledBSDF(const std::shared_ptr<Texture>& albedoTexture, float roughness = 0.1f, float metallic = 0.0f);
    PrincipledBSDF(const std::shared_ptr<Texture>& albedoTexture, const std::shared_ptr<Texture>& roughnessTexture, float metallic = 0.0f);
    PrincipledBSDF(const std::shared_ptr<Texture>& albedoTexture, const std::shared_ptr<Texture>& roughnessTexture, const std::shared_ptr<Texture>& metallicTexture);
    PrincipledBSDF(const std::shared_ptr<Texture>& albedoTexture, const std::shared_ptr<Texture>& roughnessTexture, float metallic, const std::shared_ptr<Texture>& normalTexture, const TextureTransform& transform = TextureTransform());
    PrincipledBSDF(const std::shared_ptr<Texture>& albedoTexture, const std::shared_ptr<Texture>& roughnessTexture, const std::shared_ptr<Texture>& metallicTexture, const std::shared_ptr<Texture>& normalTexture, const TextureTransform& transform = TextureTransform());
    PrincipledBSDF(const std::shared_ptr<Texture>& albedoTexture, float roughness, float metallic, const std::shared_ptr<Texture>& normalTexture, const TextureTransform& transform = TextureTransform());
    PrincipledBSDF(const std::shared_ptr<Texture>& albedoTexture, float roughness, float metallic, const TextureTransform& transform = TextureTransform());
    // Yeni constructor
    PrincipledBSDF(const std::shared_ptr<Texture>& albedoTexture, const std::shared_ptr<Texture>& opacityTexture, float roughness = 0.1f, float metallic = 0.0f);

    virtual bool hasTexture() const override;
    virtual double getIndexOfRefraction() const override ;
    virtual std::shared_ptr<Texture> getTexture() const override;
    bool useSmartUVProjection = false; // Yeni üye
   

   

    bool has_normal_map() const override { return normalProperty.texture != nullptr; }
    Vec3 get_normal_from_map(double u, double v) const override;
    float get_normal_strength() const override { return normalProperty.intensity; }

   

    void setTextureTransform(const TextureTransform& transform);
    void setTilingFactor(const Vec2& factor) { tilingFactor = factor; }
    void set_normal_map(std::shared_ptr<Texture> normalMap, float normalStrength = 1.0f);

    // New methods for PBR
    void setSpecular(const Vec3& specular, float intensity = 1.0f);
    void setSpecularTexture(const std::shared_ptr<Texture>& tex, float intensity = 1.0f);
    void setEmission(const Vec3& emission, const float intensity);
    void setEmissionTexture(const std::shared_ptr<Texture>& tex, float intensity);
    void setClearcoat(float clearcoat, float clearcoatRoughness = 0.1f);
    void setAnisotropic(float anisotropic, const Vec3& anisotropicDirection);
    void setSubsurfaceScattering(const Vec3& sssColor, float sssRadius);
    Vec3 getTextureColor(double u, double v) const;
    virtual bool hasOpacityTexture() const override;
    Vec2 applyTextureTransform(double u, double v) const;
    void setMetallic(float metallic, float intensity = 1.0f);
    void setMetallicTexture(const std::shared_ptr<Texture>& tex, float intensity = 1.0f);
   
    // Yeni metodlar
    void setOpacityTexture(const std::shared_ptr<Texture>& tex, float intensity = 1.0f);
    float get_roughness(float u, float v) const {
        return getPropertyValue(roughnessProperty, Vec2(u, v)).x;
    }
    float getOpacity(const Vec2& uv) const;
    virtual bool scatter(const Ray& r_in, const HitRecord& rec, Vec3SIMD& attenuation, Ray& scattered) const override;
    float get_scattering_factor() const override {
        // Example formula incorporating reflectivity and roughness
        return 0.01f;
    }
    MaterialProperty opacityProperty;
    MaterialProperty albedoProperty;
    MaterialProperty roughnessProperty;
    MaterialProperty metallicProperty;
    MaterialProperty normalProperty;
    Vec2 tilingFactor;
    TextureTransform textureTransform; 
    MaterialProperty specularProperty;
    MaterialProperty emissionProperty;
   
    
private:

    float subsurfaceRadius = 0.0f;
    float clearcoat=0.0f;
    float clearcoatRoughness=0.0f;
    float anisotropic=0;
    Vec3SIMD anisotropicDirection;
    Vec3SIMD subsurfaceColor = Vec3SIMD(0, 0, 0);

     
    // Helper methods
    float max(float a, float b) const { return a > b ? a : b; }
    UVData transformUV(double u, double v) const;
    Vec2 applyWrapMode(const UVData& uvData) const;    
    Vec2 applyPlanarWrapping(double u, double v) const;
  
    Vec2 applyRepeatWrapping(const Vec2& uv) const;
    Vec2 applyMirrorWrapping(const Vec2& uv) const;
    Vec2 applyClampWrapping(const Vec2& uv) const;
    Vec2 applyPlanarWrapping(const Vec2& uv) const;
    Vec2 applyCubicWrapping(const Vec2& uv) const;

    Vec3SIMD fresnelSchlick(float cosTheta, const Vec3SIMD& F0) const;

    Vec3SIMD fresnelSchlickRoughness(float cosTheta, const Vec3SIMD& F0, float roughness) const;

    double calculate_sss_density(double distance) const;

    double calculate_sss_absorption(double distance) const;

    Vec3SIMD calculate_sss_attenuation(double distance) const;

    Vec3SIMD sample_henyey_greenstein(const Vec3SIMD& wi, double g) const;
    // Existing methods
    virtual MaterialType type() const override { return MaterialType::PrincipledBSDF; }
    
    Vec3SIMD computeScatterDirection(const Vec3SIMD& N, const Vec3SIMD& T, const Vec3SIMD& B, float roughness) const;
    void createCoordinateSystem(const Vec3SIMD& N, Vec3SIMD& T, Vec3SIMD& B) const;
    Vec3SIMD reflect(const Vec3SIMD& v, const Vec3SIMD& n) const;
   
  
    Vec3SIMD computeFresnel(const Vec3SIMD& F0, float cosTheta) const;
    float DistributionGGX(const Vec3SIMD& N, const Vec3SIMD& H, float roughness) const;
    float GeometrySchlickGGX(float NdotV, float roughness) const;
    float GeometrySmith(const Vec3SIMD& N, const Vec3SIMD& V, const Vec3SIMD& L, float roughness) const;
  
    Vec3 getPropertyValue(const MaterialProperty& prop, const Vec2& uv) const;
    Vec2 applyTiling(double u, double v) const;
    Vec3SIMD importanceSampleGGX(float u1, float u2, float roughness, const Vec3SIMD& N) const;
   
  
    // New helper methods for PBR
    Vec3SIMD computeSubsurfaceScattering(const Vec3SIMD& N, const Vec3SIMD& V, float thickness) const;
    Vec3SIMD computeAnisotropicDirection(const Vec3SIMD& N, const Vec3SIMD& T, const Vec3SIMD& B, float roughness, float anisotropy) const;
  
    float computeAmbientOcclusion(const Vec3SIMD& point, const Vec3SIMD& normal) const;
    Vec3SIMD albedoValue;
};