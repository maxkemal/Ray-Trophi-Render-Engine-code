#include "Metal.h"
#include "Ray.h"
#include "Texture.h"
#include <algorithm>
#include "HittableList.h"

// Constructor with Vec3 albedo
Metal::Metal(const Vec3& albedo, float roughness, float metallic, float fuzz, float clearcoat)
    : albedoProperty(albedo), roughnessProperty(roughness), metallicProperty(metallic), fuzz(fuzz), clearcoat(clearcoat), clearcoatRoughness(0.1f), specularColor(Vec3(1.0f)), specularIntensity(1.0f), anisotropic(0.0f), anisotropicDirection(Vec3(1, 0, 0)) {}

// Constructor with Texture
Metal::Metal(const std::shared_ptr<Texture>& albedoTexture, float roughness, float metallic, float fuzz, float clearcoat)
    : albedoProperty(Vec3(1.0f), 1.0f, albedoTexture), roughnessProperty(roughness), metallicProperty(metallic), fuzz(fuzz), clearcoat(clearcoat), clearcoatRoughness(0.1f), specularColor(Vec3(1.0f)), specularIntensity(1.0f), anisotropic(0.0f), anisotropicDirection(Vec3(1, 0, 0)) {}

MaterialType Metal::type() const {
    return MaterialType::Metal;
}

Vec3SIMD Metal::emitted(double u, double v, const Vec3SIMD& p) const {
    return Vec3SIMD(0, 0, 0);
}
double Metal::getIndexOfRefraction() const {
    // Metaller için genellikle kompleks kýrýlma indeksi kullanýlýr,
    // ancak basitlik için sabit bir deðer döndürebiliriz.
    return 0.0; // veya baþka bir uygun deðer
}
Vec3 Metal::getPropertyValue(const MaterialProperty& prop, const Vec2& uv) const {
    if (prop.texture) {
        return prop.texture->get_color(uv.u, uv.v) * prop.intensity;
    }
    return prop.color * prop.intensity;
}
void Metal::setSpecular(const Vec3& specular, float intensity) {
    specularProperty = MaterialProperty(specular, intensity);
}
void Metal::setMetallic(float metallic, float intensity) {
    metallicProperty.color = Vec3(metallic, metallic, metallic);
    metallicProperty.intensity = intensity;
    metallicProperty.texture = nullptr;
}
// scatter function updated to use GGX BRDF
Vec2 Metal::applyTextureTransform(double u, double v) const {
    // Transform and wrap texture coordinates
    u -= 0.5;
    v -= 0.5;
    u *= textureTransform.scale.u;
    v *= textureTransform.scale.v;

    double rotation_radians = textureTransform.rotation_degrees * M_PI / 180.0;
    double cosTheta = std::cos(rotation_radians);
    double sinTheta = std::sin(rotation_radians);
    double newU = u * cosTheta - v * sinTheta;
    double newV = u * sinTheta + v * cosTheta;
    u = newU;
    v = newV;

    u += 0.5;
    v += 0.5;
    u += textureTransform.translation.u;
    v += textureTransform.translation.v;

    u = std::fmod(u * textureTransform.tilingFactor.u, 1.0);
    v = std::fmod(v * textureTransform.tilingFactor.v, 1.0);

    return applyWrapMode(u, v);
}
void Metal::set_normal_map(std::shared_ptr<Texture> normalMap, float normalStrength) {
    normalProperty.texture = normalMap;
    normalProperty.intensity = normalStrength;
}
Vec2 Metal::applyWrapMode(double u, double v) const {
    switch (textureTransform.wrapMode) {
    case WrapMode::Repeat:
        u = std::fmod(u, 1.0);
        v = std::fmod(v, 1.0);
        if (u < 0) u += 1.0;
        if (v < 0) v += 1.0;
        break;
    case WrapMode::Mirror:
        u = std::fmod(u, 2.0);
        v = std::fmod(v, 2.0);
        if (u < 0) u += 2.0;
        if (v < 0) v += 2.0;
        if (u > 1.0) u = 2.0 - u;
        if (v > 1.0) v = 2.0 - v;
        break;
    case WrapMode::Clamp:
        u = std::max(0.0, std::min(1.0, u));
        v = std::max(0.0, std::min(1.0, v));
        break;
    }
    return Vec2(u, v);
}

float Metal::computeAmbientOcclusion(const Vec3& point, const Vec3& normal) const {
    int numSamples = 16; // Number of samples for AO calculation
    float aoRadius = 0.5f; // Radius of the AO sampling sphere
    float occlusion = 0.0f;
    HittableList world;
    for (int i = 0; i < numSamples; ++i) {
        // Generate a random direction in the hemisphere around the normal
        Vec3 randomDir = Vec3SIMD::random_in_hemisphere(normal);

        // Create a small offset to avoid self-intersection
        Vec3 samplePoint = point + normal * 0.001f;

        // Cast a ray from the sample point in the random direction
        Ray aoRay(samplePoint, randomDir);

        // Check if the ray intersects any geometry
        HitRecord tempRec;
        if (world.hit(aoRay, 0.001f, aoRadius, tempRec)) {
            occlusion += 1.0f;
        }
    }

    // Compute the final ambient occlusion factor
    float ao = 1.0f - (occlusion / float(numSamples));
    return ao;
}


bool Metal::scatter(const Ray& r_in, const HitRecord& rec, Vec3SIMD& attenuation, Ray& scattered) const {
    Vec3SIMD N = rec.normal;
    Vec3SIMD V = -r_in.direction.normalize();
    Vec2 transformedUV = applyTextureTransform(rec.u, rec.v);
    Vec3SIMD baseColor = getPropertyValue(albedoProperty, Vec2(rec.u, rec.v));
    float metallicValue = getPropertyValue(metallicProperty, Vec2(rec.u, rec.v)).x;
    float roughnessValue = max(getPropertyValue(roughnessProperty, Vec2(rec.u, rec.v)).x, 0.01f);

    // Use metallicColor to influence F0
    Vec3SIMD F0 = Vec3SIMD::lerp(Vec3(0.04f), baseColor * metallicColor, metallicValue);

    Vec3SIMD R = Vec3SIMD::reflect(-V, N);
    Vec3 scatteredDirection = R + (roughnessValue + 0.001f) * Vec3SIMD::random_in_unit_sphere();
    scattered = Ray(rec.point, scatteredDirection.normalize());
    Vec3SIMD L = scattered.direction;
    Vec3SIMD H = (V + L).normalize();

    double NDF = DistributionGGX(N, H, roughnessValue);
    double G = GeometrySmith(N, V, L, roughnessValue);
    Vec3 F = fresnelSchlick(max(Vec3::dot(H, V), 0.0), F0);

    Vec3 numerator = NDF * G * F;
    double denominator = 4.0 * max(Vec3::dot(N, V), 0.0) * max(Vec3::dot(N, L), 0.0) + 0.000001;
    Vec3 specular = numerator / denominator*2;

    Vec3SIMD kS = F;
    Vec3SIMD kD = Vec3(1.0f - metallicValue) * (Vec3(1.0) - F);

    double NdotL = max(Vec3::dot(N, L), 0.0);

    // Incorporate metallicColor into both diffuse and specular components
    Vec3SIMD diffuse = kD * baseColor / M_PI;
    Vec3SIMD spec = kS * specular * baseColor * metallicColor;

    // Blend between diffuse and specular based on metallicValue
    attenuation = Vec3SIMD::lerp(diffuse, spec, metallicValue) * NdotL;

    Vec3SIMD emissionValue = getPropertyValue(emissionProperty, transformedUV);
    attenuation += emissionValue;

    double ao = computeAmbientOcclusion(rec.point, N);
    attenuation *= ao;

    if (clearcoat > 0.0) {
        Vec3SIMD clearcoatReflection = computeClearcoat(R, N);
        double clearcoatFactor = clearcoat * (1.0 - metallicValue);
        attenuation = Vec3SIMD::lerp(attenuation, clearcoatReflection * baseColor * metallicColor, clearcoatFactor);
    }

    //attenuation = attenuation.clamp(0.0, 1.0);
    return true;
}

float Metal::DistributionGGX(const Vec3& N, const Vec3& H, float roughness) const {
    double a = roughness * roughness;
    double a2 = a * a;
    double NdotH = max(Vec3::dot(N, H), 0.0);
    double NdotH2 = NdotH * NdotH;

    double nom = a2;
    double denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = M_PI * denom * denom + 0.00000001; // Epsilon deðerini artýrýn

    return nom / std::max(denom, 0.00000001);
}

float Metal::GeometrySchlickGGX(float NdotV, float roughness) const {
    float r = (roughness + 1.0);
    float k = (r * r) / 2.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / std::max(denom, 0.0000001f);
}

float Metal::GeometrySmith(const Vec3& N, const Vec3& V, const Vec3& L, float roughness) const {
    float NdotV = max(Vec3::dot(N, V), 0.0f);
    float NdotL = max(Vec3::dot(N, L), 0.0f);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

Vec3 Metal::fresnelSchlick(float cosTheta, const Vec3& F0) const {
    return F0 + (Vec3(1.0) - F0) * std::pow(1.0 - cosTheta, 5.0);
}

Vec3 Metal::computeClearcoat(const Vec3& R, const Vec3& N) const {
    Vec3 clearcoatColor = Vec3(1.0);
    float clearcoatRough = clearcoatRoughness;
    Vec3 H = (R + N).normalize();
    float clearcoatNDF = DistributionGGX(N, H, clearcoatRough);
    float clearcoatG = GeometrySmith(N, R, N, clearcoatRough);
    Vec3 clearcoatF = fresnelSchlick(max(Vec3::dot(H, N), 0.0f), clearcoatColor);
    float clearcoatDenom = 4.0f * max(Vec3::dot(N, R), 0.0f) * max(Vec3::dot(N, N), 0.0f) + 0.0001f;

    return (clearcoatNDF * clearcoatG * clearcoatF) / clearcoatDenom;
}



void Metal::setEmission(const Vec3& emission, float intensity) {
    emissionProperty = MaterialProperty(emission, intensity);
}

void Metal::setEmissionTexture(const std::shared_ptr<Texture>& tex, float intensity) {
    emissionProperty = MaterialProperty(Vec3(1, 1, 1), intensity, tex);
}

void Metal::setSpecularTexture(const std::shared_ptr<Texture>& tex, float intensity) {
    specularProperty = MaterialProperty(Vec3(1, 1, 1), intensity, tex);
}

void Metal::setClearcoat(float clearcoat, float clearcoatRoughness, const Vec3& clearcoatColor) {
    this->clearcoat = clearcoat;
    this->clearcoatRoughness = clearcoatRoughness;
    this->clearcoatColor = clearcoatColor;
}

void Metal::setMetallic(float metallic, float intensity, const Vec3& color) {
    this->metallic = metallic;
    this->metallicProperty.intensity = intensity;
    this->metallicColor = color;
}

void Metal::setMetallicTexture(const std::shared_ptr<Texture>& tex, float intensity) {
    metallicProperty.texture = tex;
    metallicProperty.intensity = intensity;
}

void Metal::setAnisotropic(float anisotropic, const Vec3& anisotropicDirection) {
    this->anisotropic = anisotropic;
    this->anisotropicDirection = anisotropicDirection.normalize();
}

Vec3 Metal::computeAnisotropicDirection(const Vec3& N, const Vec3& T, const Vec3& B, float roughness, float anisotropy) const {
    float phi = 2 * M_PI * random_double();
    float cosTheta = std::pow(1 - random_double(), 1 / (roughness * anisotropy + 1));
    float sinTheta = std::sqrt(1 - cosTheta * cosTheta);

    Vec3 anisotropicDirection = sinTheta * std::cos(phi) * T + sinTheta * std::sin(phi) * B + cosTheta * N;
    return anisotropicDirection;
}

void Metal::createCoordinateSystem(const Vec3& N, Vec3& T, Vec3& B) const {
    if (std::fabs(N.x) > std::fabs(N.y)) {
        T = Vec3(N.z, 0, -N.x).normalize();
    }
    else {
        T = Vec3(0, -N.z, N.y).normalize();
    }
    B = Vec3::cross(N, T);
}



// Other helper method implementations... bu sýnýfta roughness deðeri sýfýr olunca siyah mat bir yüzey oluyor