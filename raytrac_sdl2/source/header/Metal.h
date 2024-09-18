#ifndef METAL_H
#define METAL_H

#include "Material.h"
#include "Texture.h"
#include "Vec2.h"
#include "Vec3.h"
#include <memory>




class Metal : public Material {
public:
    struct TextureTransform {
        Vec2 scale{ 1.0, 1.0 };
        double rotation_degrees{ 0.0 };
        Vec2 translation{ 0.0, 0.0 };
        Vec2 tilingFactor{ 1.0, 1.0 };
        WrapMode wrapMode{ WrapMode::Repeat };
    };

    Metal(const Vec3& albedo, float roughness, float metallic, float fuzz, float clearcoat);
        
    Metal(const std::shared_ptr<Texture>& albedoTexture, float roughness, float metallic, float fuzz, float clearcoat);
      

    MaterialType type() const override;
    virtual Vec3SIMD emitted(double u, double v, const Vec3SIMD& p) const override;
   
    virtual bool scatter(const Ray& r_in, const HitRecord& rec, Vec3SIMD& attenuation, Ray& scattered) const override;
    virtual double getIndexOfRefraction() const override;
  

   

    void setTextureTransform(const TextureTransform& transform);
    void setTilingFactor(const Vec2& factor) { tilingFactor = factor; }
    void set_normal_map(std::shared_ptr<Texture> normalMap, float normalStrength = 1.0f);

    

    // New methods for PBR
    void setSpecular(const Vec3& specular, float intensity = 1.0f);
    void setMetallic(float metallic, float intensity);
    void setSpecularTexture(const std::shared_ptr<Texture>& tex, float intensity = 1.0f);
    void setClearcoat(float clearcoat, float clearcoatRoughness = 0.1f, const Vec3& clearcoatColor = Vec3(1.0f, 1.0f, 1.0f));
    void setMetallic(float metallic, float intensity, const Vec3& color);
    void setAnisotropic(float anisotropic, const Vec3& anisotropicDirection = Vec3(1, 0, 0));
   
    void setMetallicTexture(const std::shared_ptr<Texture>& tex, float intensity = 1.0f);
    void setEmission(const Vec3& emission, float intensity = 0.0f);
    void setEmissionTexture(const std::shared_ptr<Texture>& tex, float intensity = 1.0f);
    Vec2 applyTextureTransform(double u, double v) const;
    float get_scattering_factor() const override {
        // Example formula incorporating reflectivity and roughness
        return  0.01f;
    }
    // New properties for PBR
    MaterialProperty specularProperty;
    Vec3 metallicColor;
    Vec3 albedo;
    double fuzz;
    Vec3 specularColor;    // Specular rengi
    float specularIntensity; // Specular yoðunluðu
    float clearcoat; // Clearcoat etkisi
    float clearcoatRoughness; // Clearcoat pürüzlülüðü
    float anisotropic; // Anizotropik etkiler
    Vec3 anisotropicDirection; // Anizotropik yön
    float metallic; // Metalik özellik
    float roughness;
    Vec3 clearcoatColor;
    MaterialProperty albedoProperty;
    MaterialProperty roughnessProperty;
    MaterialProperty metallicProperty;
    MaterialProperty normalProperty;
    MaterialProperty emissionProperty;
    Vec2 tilingFactor;
    TextureTransform textureTransform;
private:
   

  
    // Helper methods
    float max(float a, float b) const { return a > b ? a : b; }
    Vec2 applyWrapMode(double u, double v) const;
    float computeAmbientOcclusion(const Vec3& point, const Vec3& normal) const;
    Vec3 computeClearcoat(const Vec3& reflected, const Vec3& normal) const;
    Vec3 computeScatterDirection(const Vec3& N, const Vec3& T, const Vec3& B, float roughness) const;
    void createCoordinateSystem(const Vec3& N, Vec3& T, Vec3& B) const;
   
   
    Vec3 computeFresnel(const Vec3& F0, float cosTheta) const;
    Vec3 getPropertyValue(const MaterialProperty& prop, const Vec2& uv) const;
    Vec2 applyTiling(double u, double v) const;
   
    // GGX BRDF için yeni yardýmcý fonksiyonlar
    float DistributionGGX(const Vec3& N, const Vec3& H, float roughness) const;
    float GeometrySchlickGGX(float NdotV, float roughness) const;
    float GeometrySmith(const Vec3& N, const Vec3& V, const Vec3& L, float roughness) const;
    Vec3 fresnelSchlick(float cosTheta, const Vec3& F0) const;
    // New helper methods for PBR
    Vec3 computeAnisotropicDirection(const Vec3& N, const Vec3& T, const Vec3& B, float roughness, float anisotropy) const;
};

#endif // METAL_H
