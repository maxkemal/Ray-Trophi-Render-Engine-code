#include "PrincipledBSDF.h"
#include <cmath>
#include "Matrix4x4.h"
#include "HittableList.h"

PrincipledBSDF::PrincipledBSDF(const Vec3& albedo, float roughness, float metallic)
    : albedoProperty(albedo), roughnessProperty(Vec3(roughness)), emissionProperty{ Vec3(0.0f, 0.0f, 0.0f), 0.0f }, subsurfaceColor(Vec3(0, 0, 0)),
    subsurfaceRadius(0.0f), metallicProperty(Vec3(metallic)), clearcoat(0.0f), clearcoatRoughness(0.0f){}

PrincipledBSDF::PrincipledBSDF(const std::shared_ptr<Texture>& albedoTexture, float roughness, float metallic)
    : albedoProperty(Vec3(1, 1, 1), 1.0f, albedoTexture),
    roughnessProperty(Vec3(roughness)),
    emissionProperty{ Vec3(0.0f, 0.0f, 0.0f), 0.0f },
    metallicProperty(Vec3(metallic)),
    subsurfaceColor(Vec3(0, 0, 0)),
    subsurfaceRadius(0.0f), clearcoat(0.0f), clearcoatRoughness(0.0f),
    tilingFactor(1, 1) {}
PrincipledBSDF::PrincipledBSDF(const std::shared_ptr<Texture>& albedoTexture, const std::shared_ptr<Texture>& roughnessTexture, float metallic)
    : albedoProperty(Vec3(1, 1, 1), 1.0f, albedoTexture),
    roughnessProperty(Vec3(0.5f), 1.0f, roughnessTexture), 
    emissionProperty{ Vec3(0.0f, 0.0f, 0.0f), 0.0f },
    metallicProperty(Vec3(metallic)),
    subsurfaceColor(Vec3(0, 0, 0)),
    subsurfaceRadius(0.0f), clearcoat(0.0f), clearcoatRoughness(0.0f),
    tilingFactor(1, 1) {}
PrincipledBSDF::PrincipledBSDF(const std::shared_ptr<Texture>& albedoTexture, const std::shared_ptr<Texture>& roughnessTexture, const std::shared_ptr<Texture>& metallicTexture)
    : albedoProperty(Vec3(1, 1, 1), 1.0f, albedoTexture),
    roughnessProperty(Vec3(0.5f), 1.0f, roughnessTexture), 
    emissionProperty{ Vec3(0.0f, 0.0f, 0.0f), 0.0f },
    subsurfaceColor(Vec3(0, 0, 0)),
    subsurfaceRadius(0.0f), clearcoat(0.0f), clearcoatRoughness(0.0f),
    metallicProperty(Vec3(0.0f), 1.0f, metallicTexture) {}
PrincipledBSDF::PrincipledBSDF(const std::shared_ptr<Texture>& albedoTexture, const std::shared_ptr<Texture>& roughnessTexture, float metallic, const std::shared_ptr<Texture>& normalTexture, const TextureTransform& transform)
    : albedoProperty(Vec3(1, 1, 1), 1.0f, albedoTexture),
    roughnessProperty(Vec3(0.5f), 1.0f, roughnessTexture), 
    emissionProperty{ Vec3(0.0f, 0.0f, 0.0f), 0.0f },
    metallicProperty(Vec3(metallic)),
    normalProperty(Vec3(0.5, 0.5, 1.0), 1.0f, normalTexture),
    subsurfaceColor(Vec3(0, 0, 0)),
    subsurfaceRadius(0.0f), clearcoat(0.0f), clearcoatRoughness(0.0f),
    textureTransform(transform) {}
PrincipledBSDF::PrincipledBSDF(const std::shared_ptr<Texture>& albedoTexture, const std::shared_ptr<Texture>& roughnessTexture, const std::shared_ptr<Texture>& metallicTexture, const std::shared_ptr<Texture>& normalTexture, const TextureTransform& transform)
    : albedoProperty(Vec3(1, 1, 1), 1.0f, albedoTexture),
    roughnessProperty(Vec3(0.5f), 1.0f, roughnessTexture),
    emissionProperty{ Vec3(0.0f, 0.0f, 0.0f), 0.0f },
    metallicProperty(Vec3(0.0f), 1.0f, metallicTexture),
    normalProperty(Vec3(0.5, 0.5, 1.0), 1.0f, normalTexture),
    subsurfaceColor(Vec3(0, 0, 0)),
    subsurfaceRadius(0.0f), clearcoat(0.0f), clearcoatRoughness(0.0f),
    textureTransform(transform) {}
PrincipledBSDF::PrincipledBSDF(const std::shared_ptr<Texture>& albedoTexture, float roughness, float metallic, const std::shared_ptr<Texture>& normalTexture, const TextureTransform& transform)

    : albedoProperty(Vec3(1, 1, 1), 1.0f, albedoTexture),
    roughnessProperty(Vec3(roughness)), 
    emissionProperty{ Vec3(0.0f, 0.0f, 0.0f), 0.0f },
    metallicProperty(Vec3(metallic)),
    normalProperty(Vec3(0.5, 0.5, 1.0), 1.0f, normalTexture),
    subsurfaceColor(Vec3(0, 0, 0)),
    subsurfaceRadius(0.0f), clearcoat(0.0f), clearcoatRoughness(0.0f),
    textureTransform(transform) {}
PrincipledBSDF::PrincipledBSDF(const std::shared_ptr<Texture>& albedoTexture, float roughness, float metallic, const TextureTransform& transform)

    : albedoProperty(Vec3(1, 1, 1), 1.0f, albedoTexture),
    roughnessProperty(Vec3(roughness)), 
    metallicProperty(Vec3(metallic)),
    emissionProperty{ Vec3(0.0f, 0.0f, 0.0f), 0.0f },
    subsurfaceColor(Vec3(0, 0, 0)),
    subsurfaceRadius(0.0f), clearcoat(0.0f), clearcoatRoughness(0.0f),
    textureTransform(transform) {}
    // Initialize other properties with default values

PrincipledBSDF::PrincipledBSDF()
    : textureTransform{ Vec2(1.0, 1.0), 0.0, Vec2(1.0, 1.0) } {}
PrincipledBSDF::PrincipledBSDF(const std::shared_ptr<Texture>& albedoTexture, const std::shared_ptr<Texture>& opacityTexture, float roughness, float metallic)
    : albedoProperty{ Vec3(1, 1, 1), 1.0f, albedoTexture },
    roughnessProperty{ Vec3(1, 1, 1), roughness, nullptr },
    emissionProperty{ Vec3(0.0f, 0.0f, 0.0f), 0.0f },
    metallicProperty{ Vec3(1, 1, 1), metallic, nullptr },   
    subsurfaceColor(Vec3(0, 0, 0)),
    subsurfaceRadius(0.0f), clearcoat(0.0f), clearcoatRoughness(0.0f),
    opacityProperty{ Vec3(1, 1, 1), 1.0f, opacityTexture } {}

double PrincipledBSDF::getIndexOfRefraction() const {
    // Metaller için genellikle kompleks kýrýlma indeksi kullanýlýr,
    // ancak basitlik için sabit bir deðer döndürebiliriz.
    return 0.0; // veya baþka bir uygun deðer
}
bool PrincipledBSDF::hasTexture() const {
    return albedoProperty.texture != nullptr ||
        roughnessProperty.texture != nullptr ||
        metallicProperty.texture != nullptr ||
        normalProperty.texture != nullptr;
}


// Ya da setter metodu ile:
void PrincipledBSDF::set_normal_map(std::shared_ptr<Texture> normalMap, float normalStrength) {
    normalProperty.texture = normalMap;
    normalProperty.intensity = normalStrength;
}
Vec3 PrincipledBSDF::get_normal_from_map(double u, double v) const {
    if (normalProperty.texture) {
        Vec2 Transform = applyTextureTransform(u, v);
        return normalProperty.texture->get_color(Transform.u, Transform.v);
    }
    return Vec3(0, 0, 1);
}
void PrincipledBSDF::setSpecular(const Vec3& specular, float intensity) {
    specularProperty = MaterialProperty(specular, intensity);
}
void PrincipledBSDF::setMetallic(float metallic, float intensity) {
    metallicProperty.color = Vec3(metallic, metallic, metallic);
    metallicProperty.intensity = intensity;
    metallicProperty.texture = nullptr;
}

void PrincipledBSDF::setMetallicTexture(const std::shared_ptr<Texture>& tex, float intensity) {
    metallicProperty.texture = tex;
    metallicProperty.intensity = intensity;
}
void PrincipledBSDF::setSpecularTexture(const std::shared_ptr<Texture>& tex, float intensity) {
    specularProperty = MaterialProperty(Vec3(1, 1, 1), intensity, tex);
}

void PrincipledBSDF::setEmission(const Vec3& emission, float intensity) {
    emissionProperty = MaterialProperty(emission, intensity);
}

void PrincipledBSDF::setEmissionTexture(const std::shared_ptr<Texture>& tex, float intensity = 0.0f) {
    emissionProperty = MaterialProperty(Vec3(0, 0, 0), intensity, tex);
}

void PrincipledBSDF::setClearcoat(float clearcoat, float clearcoatRoughness) {
    this->clearcoat = clearcoat;
    this->clearcoatRoughness = clearcoatRoughness;
}

void PrincipledBSDF::setAnisotropic(float anisotropic = 0.0f, const Vec3& anisotropicDirection = Vec3(0, 0, 0)) {
    this->anisotropic = anisotropic;
    this->anisotropicDirection = anisotropicDirection.normalize();
}

void PrincipledBSDF::setSubsurfaceScattering(const Vec3& sssColor, float sssRadius) {
    this->subsurfaceColor = sssColor;
    this->subsurfaceRadius = sssRadius;
}
std::shared_ptr<Texture> PrincipledBSDF::getTexture() const {
    if (albedoProperty.texture) return albedoProperty.texture;
    if (roughnessProperty.texture) return roughnessProperty.texture;
    if (metallicProperty.texture) return metallicProperty.texture;
    if (normalProperty.texture) return normalProperty.texture;
    return nullptr;
}

Vec3 PrincipledBSDF::getTextureColor(double u, double v) const {
    std::cout << "Original UV: (" << u << ", " << v << ")" << std::endl;
    UVData uvData = transformUV(u, v);
    std::cout << "Transformed UV: (" << uvData.transformed.u << ", " << uvData.transformed.v << ")" << std::endl;
    Vec2 finalUV = applyWrapMode(uvData);
    std::cout << "Final UV: (" << finalUV.u << ", " << finalUV.v << ")" << std::endl;
    return texture->get_color(finalUV.u, finalUV.v);
}

bool PrincipledBSDF::hasOpacityTexture() const {
    return opacityProperty.texture != nullptr;
}

void PrincipledBSDF::setOpacityTexture(const std::shared_ptr<Texture>& tex, float intensity) {
    opacityProperty.texture = tex;
    opacityProperty.intensity = intensity;
}

float PrincipledBSDF::getOpacity(const Vec2& uv) const {
    if (!opacityProperty.texture) {
      //  std::cout << "getOpacity: Texture yok, 1.0 döndürülüyor" << std::endl;
          return 1.0f;
    }

    const Texture& opacityTexture = *opacityProperty.texture;

    float alpha = opacityTexture.get_alpha(uv.x, uv.y);
    Vec3 opacityColor = opacityTexture.get_color(uv.x, uv.y);

    std::cout << "getOpacity: Alpha = " << alpha << std::endl;
    std::cout << "getOpacity: Color = (" << opacityColor.x << ", " << opacityColor.y << ", " << opacityColor.z << ")" << std::endl;

    // Gri tonlama kontrolü
    if (std::abs(opacityColor.x - opacityColor.y) < 0.01f &&
        std::abs(opacityColor.x - opacityColor.z) < 0.01f) {
        float result = opacityColor.x * alpha * opacityProperty.intensity;
        std::cout << "getOpacity: Gri tonlama, sonuç = " << result << std::endl;
        return result;
    }
    else {
        float avgColor = (opacityColor.x + opacityColor.y + opacityColor.z) / 3.0f;
        float result = avgColor * alpha * opacityProperty.intensity;
        std::cout << "getOpacity: Renkli, sonuç = " << result << std::endl;
        return result;
    }
}

bool PrincipledBSDF::scatter(const Ray& r_in, const HitRecord& rec, Vec3SIMD& attenuation, Ray& scattered) const {
    Vec2 transformedUV = useSmartUVProjection ? Vec2(rec.u, rec.v) : applyTextureTransform(rec.u, rec.v);
    Vec3SIMD albedoValue = (getPropertyValue(albedoProperty, transformedUV));
    float roughness = getPropertyValue(roughnessProperty, transformedUV).x;
    float metallic = getPropertyValue(metallicProperty, transformedUV).x;
    Vec3SIMD specularValue = getPropertyValue(specularProperty, transformedUV);
    Vec3SIMD emissionValue = getPropertyValue(emissionProperty, transformedUV);
    Vec3SIMD N = rec.normal;
    Vec3SIMD V = -r_in.direction.normalize();
    Vec3SIMD L = scattered.direction;
    Vec3SIMD H = (V + L).normalize();

    // Opaklýk kontrolü
    float opacity = getOpacity(rec.uv);
    
    if (opacity < 1.0f) {
        // Geçirgenlik hesaplamasý (örneðin, rastgele bir deðeri opaklýkla karþýlaþtýrabilirsiniz)
        if (Vec3SIMD::random_double() > opacity) {
            return false; // Iþýn geçirgen, malzeme tarafýndan daðýtýlmýyor
        }
    }

    if (emissionValue.length() > 1)
    {
        attenuation = emissionValue;
        return true; //diðer yüzey hasaplamalarýný atla
    }

    float alpha = albedoProperty.texture ? albedoProperty.texture->get_alpha(rec.u, rec.v) : 1.0f;
    // Use metallicColor to influence F0

    Vec3SIMD F0 = Vec3SIMD::lerp(Vec3(0.04f), albedoValue*metallic, metallic);
    Vec3SIMD R = reflect(-V, N);

    Vec3SIMD scatteredDirection = R + (roughness + 0.001f) * Vec3SIMD::random_in_unit_sphere();
    if (roughness < 0.01f) {
        // Çok düþük roughness için tam yansýma
        scatteredDirection = R;
    }
    else {
        Vec3SIMD H = importanceSampleGGX(Vec3SIMD::random_double(), Vec3SIMD::random_double(), roughness, N);
        scatteredDirection = Vec3SIMD::reflect(-V, H);
    }
    scattered = Ray(rec.point, scatteredDirection.normalize());
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    Vec3SIMD F = fresnelSchlick(max((H.dot( V)), 0.0), F0);
    Vec3SIMD numerator = NDF * G * F;
    float denominator = 4.0 * max(N.dot(V), 0.0) * max(N.dot( L), 0.0f) + 0.0001f;
    Vec3SIMD specular = numerator / denominator * Vec3SIMD(2);
    Vec3SIMD kS = F;
    Vec3SIMD kD = Vec3SIMD(1.0f - metallic) * (Vec3SIMD(1.0) - F);
    double NdotL = max(N.dot(L), 0.0);
    // Incorporate metallicColor into both diffuse and specular components
    Vec3SIMD diffuse = kD * albedoValue / M_PI;
    Vec3SIMD spec = kS * specular * albedoValue * metallic;
    // Blend between diffuse and specular based on metallicValue
    attenuation = Vec3SIMD::lerp(diffuse, spec, metallic) * NdotL;

   
    float adjustedMetallic = pow(metallic, 2.2);
    // Emission deðerini ekleyin
    Vec3SIMD emission = emissionValue;
    // BRDF hesaplamasý
    Vec3SIMD brdf = (diffuse + spec * adjustedMetallic) * NdotL;
      if (roughness < 0.01f) {
        // Çok düþük roughness için sadece specular bileþeni kullan
        brdf = specular;
    } else {
        brdf = diffuse + specular;
    }
    // Attenuation'ý BRDF ve emission'ýn toplamý olarak hesaplayýn
    attenuation = brdf + emission;
    double ao = computeAmbientOcclusion(rec.point, N);

    attenuation = brdf * ao;

    Vec3SIMD scatter_direction;
    if (anisotropic > 0) {
        Vec3SIMD T, B;
        createCoordinateSystem(N, T, B);
        scatter_direction = computeAnisotropicDirection(N, T, B, roughness, anisotropic);
    }
    else {
        Vec3SIMD metal_dir = reflect(r_in.direction, N) + roughness * Vec3SIMD::random_in_unit_sphere();
        Vec3SIMD diffuse_dir = N + Vec3SIMD::random_in_unit_hemisphere(N);
        scatter_direction = Vec3SIMD::lerp(diffuse_dir, metal_dir, metallic);
    }
    if (scatter_direction.near_zero()) {
        scatter_direction = N;
    }

    scattered = Ray(rec.point, scatter_direction.normalize());
    float cosTheta = std::max(static_cast<float>((N.dot( V))), 0.0f);
    Vec3SIMD specularContribution = specularValue * F;
    Vec3SIMD diffuseContribution = albedoValue * (Vec3SIMD(1.0f, 1.0f, 1.0f) - F) * (1.0f - metallic);
    float specularWeight = std::max(metallic, 0.0f);
    attenuation = Vec3SIMD::lerp(diffuseContribution, specularContribution, specularWeight);


    if (clearcoat > 0) {
        Vec3SIMD clearcoatF = computeFresnel(Vec3SIMD(0.04f, 0.04f, 0.04f), cosTheta);
        float clearcoatStrength = clearcoat * (1.0f - roughness); // Stronger clearcoat on smoother surfaces
        attenuation = Vec3SIMD::lerp(attenuation, clearcoatF, clearcoatStrength);
    }

    if (subsurfaceRadius > 0) {
        float estimatedThickness = 0.1f;
        Vec3SIMD subsurfaceContribution = computeSubsurfaceScattering(N, V, estimatedThickness);
        attenuation += subsurfaceContribution * (0.5f - metallic);

    }
  
    // Son attenuation deðerini hesapla
    Vec3SIMD nonEmissiveAttenuation = attenuation - emission;
   
    attenuation = nonEmissiveAttenuation + emission;

    return true;
}

Vec3SIMD PrincipledBSDF::fresnelSchlickRoughness(float cosTheta, const Vec3SIMD& F0, float roughness) const {
    // Adjust the power by mixing roughness, giving a smoother transition for rough surfaces
    float exponent = std::pow(1.0f - cosTheta, 5.0f);
    return F0 + (Vec3SIMD(1.0f) - F0) * exponent * (1.0f - roughness); // Roughness controls smoothness
}

Vec3SIMD PrincipledBSDF::fresnelSchlick(float cosTheta, const Vec3SIMD& F0) const {
    return F0 + (Vec3SIMD(1.0) - F0) * std::pow(1.0 - cosTheta, 5.0);
}

// Reflection helper function
Vec3SIMD PrincipledBSDF::reflect(const Vec3SIMD& v, const Vec3SIMD& n) const {
    return v - 2 * Vec3SIMD::dot(v, n) * n;
}


Vec3SIMD PrincipledBSDF::computeAnisotropicDirection(const Vec3SIMD& N, const Vec3SIMD& T, const Vec3SIMD& B, float roughness, float anisotropy) const {
    float r1 = Vec3SIMD::random_double();
    float r2 = Vec3SIMD::random_double();

    float phi = 2 * M_PI * r1;
    float cosTheta = std::sqrt((1 - r2) / (1 + (anisotropy * anisotropy - 1) * r2));
    float sinTheta = std::sqrt(1 - cosTheta * cosTheta);

    float x = sinTheta * std::cos(phi);
    float y = sinTheta * std::sin(phi);
    float z = cosTheta;

    return (x * T + y * B + z * N);
}

float PrincipledBSDF::computeAmbientOcclusion(const Vec3SIMD& point, const Vec3SIMD& normal) const {
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

void PrincipledBSDF::createCoordinateSystem(const Vec3SIMD& N, Vec3SIMD& T, Vec3SIMD& B) const {
    if (std::fabs(N.x()) > std::fabs(N.y())) {
        T = Vec3SIMD(N.z(), 0, -N.x());
    }
    else {
        T = Vec3SIMD(0, -N.z(), N.y());
    }
    B = Vec3SIMD::cross(N, T);
}

Vec3SIMD PrincipledBSDF::computeFresnel(const Vec3SIMD& F0, float cosTheta) const {
    float p = std::pow(1.0f - cosTheta, 2.0f);
    return F0 + (Vec3SIMD(1.0f, 1.0f, 1.0f) - F0) * p;
}
Vec3SIMD PrincipledBSDF::computeSubsurfaceScattering(const Vec3SIMD& N, const Vec3SIMD& V, float thickness) const {
    double cosTheta = std::max(N.dot(V), 0.0f);
    // Iþýðýn malzeme içinden geçiþ mesafesini tahmin et
    float transmittance = std::exp(-thickness / subsurfaceRadius);
    // Malzemenin kenarlarýnda ve ince kýsýmlarýnda daha fazla ýþýk geçiþi
    float fresnel = std::pow(1.0f - cosTheta, 5.0f);
    float thinness = 0.2f - thickness;
    // SSS yoðunluðunu hesapla
    float sssIntensity = transmittance * (fresnel + thinness);
    return subsurfaceColor * sssIntensity;
}

// Optimization 2: Precompute common values
float PrincipledBSDF::DistributionGGX(const Vec3SIMD& N, const Vec3SIMD& H, float roughness) const {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(N.dot(H), 0.0f);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
    denom = M_PI * denom * denom + 0.00001f; // Slightly increased epsilon

    return nom / denom;
}

// Optimization 3: Use faster approximations for expensive operations
float PrincipledBSDF::GeometrySchlickGGX(float NdotV, float roughness) const {
    float r = (roughness + 1.0f);
    float k = (r * r) / 8.0f; // Changed from /2.0 to /8.0 for better approximation

    return NdotV / (NdotV * (1.0f - k) + k);
}

// Optimization 4: Combine multiple functions into one for better instruction pipelining
float PrincipledBSDF::GeometrySmith(const Vec3SIMD& N, const Vec3SIMD& V, const Vec3SIMD& L, float roughness) const {
    float r = (roughness + 1.0f);
    float k = (r * r) / 8.0f;

    float NdotV = max(N.dot(V), 0.0001f);
    float NdotL = max(N.dot(L), 0.0001f);

    float ggx2 = NdotV / (NdotV * (1.0f - k) + k);
    float ggx1 = NdotL / (NdotL * (1.0f - k) + k);

    return ggx1 * ggx2;
}

// Optimization 5: Use importance sampling for more efficient Monte Carlo integration
Vec3SIMD PrincipledBSDF::importanceSampleGGX(float u1, float u2, float roughness, const Vec3SIMD& N) const {
    float a = roughness * roughness;
    float phi = 2.0f * M_PI * u1;
    float cosTheta = std::sqrt((1.0f - u2) / (1.0f + (a * a - 1.0f) * u2));
    float sinTheta = std::sqrt(1.0f - cosTheta * cosTheta);

    Vec3SIMD H(sinTheta * std::cos(phi), sinTheta * std::sin(phi), cosTheta);

    Vec3SIMD up = std::abs(N.z()) < 0.999f ? Vec3SIMD(0.0f, 0.0f, 1.0f) : Vec3SIMD(1.0f, 0.0f, 0.0f);
    Vec3SIMD tangentX = up.cross(N);
    Vec3SIMD tangentY = N.cross(tangentX);

    return (tangentX * H.x() + tangentY * H.y() + N * H.z());
}

Vec3SIMD PrincipledBSDF::sample_henyey_greenstein(const Vec3SIMD& wi, double g) const {
    // Cosinüsü üretmek için rastgele bir sayý üretiyoruz
    double cos_theta = 1.0 - 2.0 * Vec3SIMD::random_double();

    // Phase function deðerini hesaplýyoruz
    double phase_value = (1.0 - g * g) / (4.0 * M_PI * pow(1.0 + g * g - 2.0 * g * cos_theta, 1.5));

    // Birim küre üzerinde rastgele bir nokta seçiyoruz
    Vec3SIMD on_unit_sphere = Vec3SIMD::random_in_unit_sphere();

    // Gelen ýþýn ve rastgele nokta ile bir düzlem oluþturuyoruz
    Vec3SIMD w = wi;
    Vec3SIMD u = Vec3SIMD::random_unit_vector().cross(w);
    Vec3SIMD v = Vec3SIMD::cross(w, u);

    // Saçýlan ýþýn vektörünü hesaplýyoruz
    Vec3SIMD scatter_direction = u * sqrt(1 - cos_theta * cos_theta) * cos(2 * M_PI * Vec3SIMD::random_double())
        + v * sqrt(1 - cos_theta * cos_theta) * sin(2 * M_PI * Vec3SIMD::random_double())
        + w * cos_theta;

    return scatter_direction;
}

Vec3 PrincipledBSDF::getPropertyValue(const MaterialProperty& prop, const Vec2& uv) const {
    if (prop.texture) {
        return prop.texture->get_color(uv.u, uv.v) * prop.intensity;
    }
    return prop.color * prop.intensity;
}

Vec2 PrincipledBSDF::applyTiling(double u, double v) const {
    return Vec2(fmod(u * tilingFactor.u, 1.0),
        fmod(v * tilingFactor.v, 1.0));
}

Vec2 PrincipledBSDF::applyTextureTransform(double u, double v) const {
    // Merkezi (0.5, 0.5) olarak kabul edelim
    u -= 0.5;
    v -= 0.5;
    // Ölçeklendirme
    u *= textureTransform.scale.u;
    v *= textureTransform.scale.v;
    // Döndürme (dereceyi radyana çevir)
    double rotation_radians = textureTransform.rotation_degrees * M_PI / 180.0;
    double cosTheta = std::cos(rotation_radians);
    double sinTheta = std::sin(rotation_radians);
    double newU = u * cosTheta - v * sinTheta;
    double newV = u * sinTheta + v * cosTheta;
    u = newU;
    v = newV;
    // Merkezi geri taþý
    u += 0.5;
    v += 0.5;
    // Öteleme uygula
    u += textureTransform.translation.u;
    v += textureTransform.translation.v;
    // Tiling uygula
    u *= textureTransform.tilingFactor.u;
    v *= textureTransform.tilingFactor.v;
    // Sarma modunu uygula
    UVData uvData = transformUV(u, v); // Orijinal ve dönüþmüþ UV'leri almak için çaðýr
    return applyWrapMode(uvData);
}

UVData PrincipledBSDF::transformUV(double u, double v) const {
    UVData uvData;
    uvData.original = Vec2(u, v);

    // Tüm dönüþümleri uygula
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

    u *= textureTransform.tilingFactor.u;
    v *= textureTransform.tilingFactor.v;

    uvData.transformed = Vec2(u, v);
    return uvData;
}


Vec2 PrincipledBSDF::applyWrapMode(const UVData& uvData) const {
    switch (textureTransform.wrapMode) {
    case WrapMode::Repeat:
        return applyRepeatWrapping(uvData.transformed);
    case WrapMode::Mirror:
        return applyMirrorWrapping(uvData.transformed);
    case WrapMode::Clamp:
        return applyClampWrapping(uvData.transformed);
    case WrapMode::Planar:
        return applyPlanarWrapping(uvData.original);
    case WrapMode::Cubic:
        return applyCubicWrapping(uvData.transformed);
    }
    return uvData.transformed;
}
Vec2 PrincipledBSDF::applyRepeatWrapping(const Vec2& uv) const {
    double u = std::fmod(uv.u, 1.0);
    double v = std::fmod(uv.v, 1.0);
    if (u < 0) u += 1.0;
    if (v < 0) v += 1.0;
    return Vec2(u, v);
}

Vec2 PrincipledBSDF::applyMirrorWrapping(const Vec2& uv) const {
    double u = std::fmod(uv.u, 2.0);
    double v = std::fmod(uv.v, 2.0);
    if (u < 0) u += 2.0;
    if (v < 0) v += 2.0;
    if (u > 1.0) u = 2.0 - u;
    if (v > 1.0) v = 2.0 - v;
    return Vec2(u, v);
}

Vec2 PrincipledBSDF::applyClampWrapping(const Vec2& uv) const {
    double u = (uv.u < 0.0) ? 0.0 : ((uv.u > 1.0) ? 1.0 : uv.u);
    double v = (uv.v < 0.0) ? 0.0 : ((uv.v > 1.0) ? 1.0 : uv.v);
    return Vec2(u, v);
}

Vec2 PrincipledBSDF::applyPlanarWrapping(const Vec2& uv) const {
    // Planer sarma için orijinal UV'leri kullan
    return uv;
}

Vec2 PrincipledBSDF::applyCubicWrapping(const Vec2& uv) const {
    // UV koordinatlarýný 0-3 aralýðýna geniþlet
    double u_scaled = uv.u * 3.0;
    double v_scaled = uv.v * 3.0;

    // Hangi yüzeyde olduðumuzu belirle
    int face = static_cast<int>(u_scaled) + 3 * static_cast<int>(v_scaled);

    // Yüzey içindeki lokal koordinatlarý hesapla
    double u_local = std::fmod(u_scaled, 1.0);
    double v_local = std::fmod(v_scaled, 1.0);

    // Yüzeye göre koordinatlarý ayarla
    switch (face % 6) {  // 6'ya göre mod alarak taþmalarý önlüyoruz
    case 0: // Ön yüz
        return Vec2(u_local, v_local);
    case 1: // Sað yüz
        return Vec2(v_local, 1.0 - u_local);
    case 2: // Arka yüz
        return Vec2(1.0 - u_local, v_local);
    case 3: // Sol yüz
        return Vec2(1.0 - v_local, 1.0 - u_local);
    case 4: // Üst yüz
        return Vec2(u_local, 1.0 - v_local);
    case 5: // Alt yüz
        return Vec2(u_local, v_local);
    }

    // Bu noktaya asla ulaþýlmamalý, ama güvenlik için ekliyoruz
    return uv;
}
void PrincipledBSDF::setTextureTransform(const TextureTransform& transform) {
    textureTransform = transform;
}
