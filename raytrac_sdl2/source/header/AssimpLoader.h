#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <memory>
#include "Triangle.h"
#include "Camera.h"
#include "Mesh.h"
#include "Material.h"
#include "PrincipledBSDF.h"
#include "Dielectric.h"
#include "Metal.h"
#include "Texture.h"
#include "AreaLight.h"
#include "Light.h"
#include "globals.h"
#include <map>
#include "Quaternion.h"

std::string baseDirectory;

struct AnimationData {
    std::string name;
    double duration;
    double ticksPerSecond;
    std::map<std::string, std::vector<aiVectorKey>> positionKeys;
    std::map<std::string, std::vector<aiQuatKey>> rotationKeys;
    std::map<std::string, std::vector<aiVectorKey>> scalingKeys;
};

static std::vector<std::shared_ptr<Camera>> cameras; // Kamera listesi

class AssimpLoader {
public:
      
   /* static std::vector<std::shared_ptr<Mesh>> loadModelToMeshes(const std::string& filename) {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cerr << "Assimp error: " << importer.GetErrorString() << std::endl;
            return {};
        }

        std::vector<std::shared_ptr<Mesh>> meshes;
        processNode(scene->mRootNode, scene, meshes);
        return meshes;
    }*/
   
    static std::vector<std::shared_ptr<Triangle>> loadModelToTriangles(const std::string& filename, const std::shared_ptr<Material>& material=0){
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs);
          //processLights(scene);
          //processCameras(scene);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cerr << "Assimp error: " << importer.GetErrorString() << std::endl;
            return {};
        }
      
        std::vector<std::shared_ptr<Triangle>> triangles;
        processNodeToTriangles(scene->mRootNode, scene, triangles);
        // Check if the scene contains animations
        if (scene->HasAnimations()) {
            try {
                // Load animation data
                std::vector<AnimationData> animations = loadAnimations(filename);

                if (animations.empty()) {
                    std::cerr << "Warning: No valid animation data found in the file: " << filename << std::endl;
                }
                else {
                    // Store the animation data if needed
                    // Example (using a hypothetical AnimationManager class):                 

                    std::cout << "Successfully loaded animations for file: " << filename << std::endl;
                }
            }
            catch (const std::exception& e) {
                std::cerr << "Error loading animations: " << e.what() << std::endl;
            }
        }
        else {
            std::cout << "No animations found in the file: " << filename << std::endl;
        }

        return triangles;
    }
    std::vector<std::shared_ptr<Light>> getLights() const {
        return lights;
    }

    std::vector<std::shared_ptr<Camera>> getCameras() const {
        return cameras;
    }

    template<typename T>
    int findKeyframeIndex(const std::vector<T>& keys, double animationTime) {
        for (size_t i = 0; i < keys.size() - 1; i++) {
            if (animationTime < keys[i + 1].mTime) {
                return static_cast<int>(i);
            }
        }
        return static_cast<int>(keys.size() - 1);
    }

    Vec3 interpolatePosition(const std::vector<aiVectorKey>& positionKeys, double time) {
        // Interpolate between the two nearest keys for the given time
        if (positionKeys.size() == 1)
            return Vec3(positionKeys[0].mValue.x, positionKeys[0].mValue.y, positionKeys[0].mValue.z);

        // Find the surrounding keys
        for (size_t i = 0; i < positionKeys.size() - 1; ++i) {
            if (time < positionKeys[i + 1].mTime) {
                double factor = (time - positionKeys[i].mTime) / (positionKeys[i + 1].mTime - positionKeys[i].mTime);
                aiVector3D start = positionKeys[i].mValue;
                aiVector3D end = positionKeys[i + 1].mValue;
                return Vec3SIMD::lerp(Vec3(start.x, start.y, start.z), Vec3(end.x, end.y, end.z), factor);
            }
        }
        return Vec3(positionKeys.back().mValue.x, positionKeys.back().mValue.y, positionKeys.back().mValue.z);
    }

    Quaternion interpolateRotation(const std::vector<aiQuatKey>& rotationKeys, double time) {
        if (rotationKeys.size() == 1)
            return Quaternion(rotationKeys[0].mValue.x, rotationKeys[0].mValue.y, rotationKeys[0].mValue.z, rotationKeys[0].mValue.w);

        for (size_t i = 0; i < rotationKeys.size() - 1; ++i) {
            if (time < rotationKeys[i + 1].mTime) {
                double factor = (time - rotationKeys[i].mTime) / (rotationKeys[i + 1].mTime - rotationKeys[i].mTime);
                aiQuaternion start = rotationKeys[i].mValue;
                aiQuaternion end = rotationKeys[i + 1].mValue;
                return Quaternion::slerp(Quaternion(start.x, start.y, start.z, start.w), Quaternion(end.x, end.y, end.z, end.w), factor);
            }
        }
        return Quaternion(rotationKeys.back().mValue.x, rotationKeys.back().mValue.y, rotationKeys.back().mValue.z, rotationKeys.back().mValue.w);
    }

    Vec3 interpolateScaling(const std::vector<aiVectorKey>& scalingKeys, double time) {
        if (scalingKeys.size() == 1)
            return Vec3(scalingKeys[0].mValue.x, scalingKeys[0].mValue.y, scalingKeys[0].mValue.z);

        for (size_t i = 0; i < scalingKeys.size() - 1; ++i) {
            if (time < scalingKeys[i + 1].mTime) {
                double factor = (time - scalingKeys[i].mTime) / (scalingKeys[i + 1].mTime - scalingKeys[i].mTime);
                aiVector3D start = scalingKeys[i].mValue;
                aiVector3D end = scalingKeys[i + 1].mValue;
                return Vec3SIMD::lerp(Vec3(start.x, start.y, start.z), Vec3(end.x, end.y, end.z), factor);
            }
        }
        return Vec3(scalingKeys.back().mValue.x, scalingKeys.back().mValue.y, scalingKeys.back().mValue.z);
    }


    static std::vector<AnimationData> loadAnimations(const std::string& filename) {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            throw std::runtime_error("Failed to load scene: " + std::string(importer.GetErrorString()));
        }

        std::vector<AnimationData> animations;
        try {
            for (unsigned int i = 0; i < scene->mNumAnimations; i++) {
                const aiAnimation* animation = scene->mAnimations[i];
                AnimationData animData;

                animData.name = animation->mName.C_Str();
                animData.duration = animation->mDuration;
                animData.ticksPerSecond = animation->mTicksPerSecond;

                for (unsigned int j = 0; j < animation->mNumChannels; j++) {
                    const aiNodeAnim* channel = animation->mChannels[j];
                    std::string nodeName = channel->mNodeName.C_Str();

                    // Position keys
                    for (unsigned int k = 0; k < channel->mNumPositionKeys; k++) {
                        animData.positionKeys[nodeName].push_back(channel->mPositionKeys[k]);
                    }

                    // Rotation keys
                    for (unsigned int k = 0; k < channel->mNumRotationKeys; k++) {
                        animData.rotationKeys[nodeName].push_back(channel->mRotationKeys[k]);
                    }

                    // Scaling keys
                    for (unsigned int k = 0; k < channel->mNumScalingKeys; k++) {
                        animData.scalingKeys[nodeName].push_back(channel->mScalingKeys[k]);
                    }
                }

                animations.push_back(animData);
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error processing animations: " << e.what() << std::endl;
        }

        return animations;
    }

private:
    static std::vector<std::shared_ptr<Light>> lights;
   

    static void processNodeToTriangles(aiNode* node, const aiScene* scene, std::vector<std::shared_ptr<Triangle>>& triangles) {
        // Her node için dünya uzayýndaki transformasyon
        aiMatrix4x4 nodeTransform = node->mTransformation;

        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            auto convertedMaterial = processMaterial(material, scene);

            // Mesh'leri doðru transform ile iþliyoruz
            processTriangles(mesh, nodeTransform, convertedMaterial, triangles);
        }

        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            processNodeToTriangles(node->mChildren[i], scene, triangles);
        }
    }

    static void processCameras(const aiScene* scene) {
        try {
            if (!scene || !scene->HasCameras()) {
                std::cout << "Sahne kameralarý içermiyor, iþlem devam ediyor..." << std::endl;
                return;  // Kamera yoksa fonksiyondan çýk
            }

            for (unsigned int i = 0; i < scene->mNumCameras; i++) {
                aiCamera* aiCam = scene->mCameras[i];
                if (!aiCam) {
                    std::cerr << "Kamera verisi eksik, bu kamera atlanacak." << std::endl;
                    continue;  // Eðer kamera verisi eksikse, bu adýmý atla
                }

                // Kamera parametrelerini al
                Vec3 lookfrom(aiCam->mPosition.x, aiCam->mPosition.y, aiCam->mPosition.z);
                Vec3 lookat(aiCam->mLookAt.x, aiCam->mLookAt.y, aiCam->mLookAt.z);
                Vec3 vup(aiCam->mUp.x, aiCam->mUp.y, aiCam->mUp.z);

                double vfov = aiCam->mHorizontalFOV;  // Varsayýlan yatay FOV, uygun olaný seçin
                double aspect = static_cast<double>(aiCam->mAspect);  // Kamera oraný (genellikle sahne bilgilerine baðlýdýr)
                double aperture = 0.0;  // Varsayýlan deðeri kullanýn veya ayarlarý güncelleyin
                double focus_dist = 1.5;  // Varsayýlan deðer
                int blade_count = 4;  // Varsayýlan deðer

                // Kamera nesnesi oluþturun
                auto camera = std::make_shared<Camera>(lookfrom, lookat, vup, vfov, aspect, aperture, focus_dist, blade_count);

                // Kamerayý listeye ekleyin
                cameras.push_back(camera);

                // Debug çýktýsý (isteðe baðlý)
              //  std::cout << "Kamera bilgileri - LookFrom: (" << lookfrom.x << ", " << lookfrom.y << ", " << lookfrom.z << ")" << std::endl;
               // std::cout << "Kamera bilgileri - LookAt: (" << lookat.x << ", " << lookat.y << ", " << lookat.z << ")" << std::endl;
               // std::cout << "Kamera bilgileri - VUp: (" << vup.x << ", " << vup.y << ", " << vup.z << ")" << std::endl;
               // std::cout << "Kamera bilgileri - FOV: " << vfov << ", Aspect: " << aspect << std::endl;
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Kamera iþleme sýrasýnda bir hata oluþtu: " << e.what() << std::endl;
        }
    }


    static void processLights(const aiScene* scene) {
        try {
            if (!scene || !scene->HasLights()) {
                std::cout << "Sahnede ýþýk bulunmuyor, iþlem devam ediyor..." << std::endl;
                return;  // Iþýk yoksa fonksiyondan çýk
            }

            for (unsigned int i = 0; i < scene->mNumLights; i++) {
                aiLight* aiLgt = scene->mLights[i];

                // Iþýk türünü ve pozisyonu yazdýrarak kontrol edin
                std::cout << "Toplam eklenen ýþýk isimleri: " << aiLgt->mName.C_Str() << std::endl;
                std::cout << "Iþýk türü: " << aiLgt->mType << ", Pozisyon: ("
                    << aiLgt->mPosition.x << ", "
                    << aiLgt->mPosition.y << ", "
                    << aiLgt->mPosition.z << ")" << std::endl;

                if (aiLgt->mType == aiLightSource_DIRECTIONAL) {
                    aiColor3D color(aiLgt->mColorDiffuse.r,
                        aiLgt->mColorDiffuse.g,
                        aiLgt->mColorDiffuse.b);

                    Vec3SIMD direction = Vec3SIMD(aiLgt->mDirection.x, aiLgt->mDirection.y, aiLgt->mDirection.z);
                    Vec3SIMD intensity = Vec3SIMD(color.r, color.g, color.b);
                    auto light = std::make_shared<DirectionalLight>(direction, intensity, 40.0);
                    lights.push_back(light);  // Iþýk vektörüne ekle
                }

                if (aiLgt->mType == aiLightSource_POINT) {
                    // Assimp ýþýðýnýn renk bilgilerini al
                    aiColor3D color(aiLgt->mColorDiffuse.r,
                        aiLgt->mColorDiffuse.g,
                        aiLgt->mColorDiffuse.b);
                    Vec3SIMD position = Vec3(aiLgt->mPosition.x, aiLgt->mPosition.y, aiLgt->mPosition.z);

                    Vec3SIMD intensity = Vec3(color.r, color.g, color.b) / 1000;
                    auto light = std::make_shared<PointLight>(position, intensity, 0.0f);
                    lights.push_back(light);  // Iþýk vektörüne ekle
                }

                if (aiLgt->mType == aiLightSource_AREA) {
                    Vec3SIMD position(aiLgt->mPosition.x, aiLgt->mPosition.y, aiLgt->mPosition.z);
                    Vec3SIMD direction(aiLgt->mDirection.x, aiLgt->mDirection.y, aiLgt->mDirection.z);
                    Vec3SIMD u(aiLgt->mUp.x, aiLgt->mUp.y, aiLgt->mUp.z);
                    Vec3SIMD v(aiLgt->mUp.x, aiLgt->mUp.y, aiLgt->mUp.z);

                    // Geniþlik ve yükseklik varsayýlan deðerleri
                    double width = 1.0;
                    double height = 1.0;

                    aiColor3D color = aiLgt->mColorDiffuse;
                    Vec3SIMD intensity(color.r, color.g, color.b);

                    auto light = std::make_shared<AreaLight>(position, u, v, width, height, intensity);
                    light->setWidth(width);
                    light->setHeight(height);
                    lights.push_back(light);
                }
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Iþýk iþleme sýrasýnda bir hata oluþtu: " << e.what() << std::endl;
        }
    }

    static void processTriangles(aiMesh* mesh, const aiMatrix4x4& transform, const std::shared_ptr<Material>& material, std::vector<std::shared_ptr<Triangle>>& triangles) {
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            if (face.mNumIndices != 3) continue; // Skip non-triangular faces

            std::vector<Vec3SIMD> vertices;
            std::vector<Vec3SIMD> normals;
            std::vector<Vec2> texCoords;

            for (unsigned int j = 0; j < 3; j++) {
                unsigned int index = face.mIndices[j];

                // Vertex'lerin dönüþümü
                aiVector3D vertex = mesh->mVertices[index];
                aiVector3D transformedVertex = transform * vertex;
                vertices.emplace_back(transformedVertex.x, transformedVertex.y, transformedVertex.z);

                // Normal'lerin dönüþümü
                if (mesh->HasNormals()) {
                    aiVector3D normal = mesh->mNormals[index];
                    aiVector3D transformedNormal = transform * normal;
                    normals.emplace_back(transformedNormal.x, transformedNormal.y, transformedNormal.z);
                }
                if (mesh->HasTextureCoords(0)) {
                    aiVector3D texCoord = mesh->mTextureCoords[0][index];
                    float u = texCoord.x;
                    float v = 1.0f - texCoord.y; // Y koordinatýný ters çevirin
                    texCoords.emplace_back(u, v);
                }
                else {
                    texCoords.emplace_back(0.0f, 0.0f);
                }
            }

            auto triangle = std::make_shared<Triangle>(
                vertices[0], vertices[1], vertices[2],
                normals[0], normals[1], normals[2],
                texCoords[0], texCoords[1], texCoords[2],
                material, mesh->mMaterialIndex // Assuming mMaterialIndex can be used as smoothGroup
            );

            triangles.push_back(triangle);
        }
    }  
    //static void processNode(aiNode* node, const aiScene* scene, std::vector<std::shared_ptr<Mesh>>& meshes) {
    //    // Process all the node's meshes (if any)
    //    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
    //        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
    //        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    //        auto convertedMaterial = processMaterial(material, scene);
    //        meshes.push_back(processMesh(mesh, scene, convertedMaterial));
    //    }
    //   
    //    // Then do the same for each of its children
    //    for (unsigned int i = 0; i < node->mNumChildren; i++) {
    //        processNode(node->mChildren[i], scene, meshes);
    //    }
    //}
    static std::string sanitizeTextureName(const aiString& str) {
        std::string textureName = str.C_Str();
        size_t pos = 0;
        while ((pos = textureName.find("%20", pos)) != std::string::npos) {
            textureName.replace(pos, 3, " ");
            pos += 1;
        }
        return textureName;
    }
   static  void processMaterialTexture(aiMaterial* aiMat, aiTextureType type, MaterialProperty& property) {
        aiString str;
        if (aiMat->GetTextureCount(type) > 0) {
            aiMat->GetTexture(type, 0, &str);
            std::string textureName = sanitizeTextureName(str);
            property.texture = loadTexture(textureName.c_str());
        }
    }

    static std::shared_ptr<Material> processMaterial(aiMaterial* aiMat, const aiScene* scene) {
        // Create the Material and assign properties
        auto material = std::make_shared<PrincipledBSDF>();  // Example material, replace with correct type as needed
        aiString str;
      
      

       // Diffuse renk ve tekstürü al
        aiColor3D color(0.0f, 0.0f, 0.0f);
      
        aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, color);       
        material->albedoProperty = MaterialProperty(Vec3SIMD(color.r, color.g, color.b), 1.0f);
       
       
        if (aiMat->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
           
            aiMat->GetTexture(aiTextureType_DIFFUSE, 0, &str);
            std::string textureName = sanitizeTextureName(str);
            material->albedoProperty.texture = loadTexture(textureName.c_str());
        }

        // Roughness
        aiColor3D colorR(0.0f, 0.0f, 0.0f);
        aiMat->Get(AI_MATKEY_ROUGHNESS_FACTOR, colorR);
        material->roughnessProperty = MaterialProperty(Vec3SIMD(colorR.r, colorR.g, colorR.b), colorR.r);
      
        // Metallic
        aiColor3D colorM(0.0f, 0.0f, 0.0f);
       
        aiMat->Get(AI_MATKEY_METALLIC_FACTOR, colorM);
        material->metallicProperty = MaterialProperty(Vec3SIMD(colorM.r, colorM.g, colorM.b), colorM.r);
      
        // Normal
        if (aiMat->GetTextureCount(aiTextureType_NORMALS) > 0) {
         
            aiMat->GetTexture(aiTextureType_NORMALS, 0, &str);
            std::string textureName = sanitizeTextureName(str);
            material->normalProperty.texture = loadTexture(textureName.c_str());
        }
 

        // Emissive Color
        float emissiveindensity=0;
        aiColor3D emissiveColor(0.0f, 0.0f, 0.0f);
        aiMat->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveColor);
        aiMat->Get(AI_MATKEY_EMISSIVE_INTENSITY, emissiveindensity);
        material->emissionProperty = MaterialProperty(Vec3SIMD(emissiveColor.r, emissiveColor.g, emissiveColor.b), emissiveindensity);
        material->setEmission(Vec3SIMD(emissiveColor.r, emissiveColor.g, emissiveColor.b), emissiveindensity);
         // Specular Reflection (Glossiness)
      
        aiColor3D specularColor(0.0f, 0.0f, 0.0f);
        aiMat->Get(AI_MATKEY_COLOR_SPECULAR, specularColor);
        material->specularProperty = MaterialProperty(Vec3SIMD(specularColor.r, specularColor.g, specularColor.b), 1.0f);
        material->setSpecular((Vec3SIMD(specularColor.r, specularColor.g, specularColor.b), 1.0f));
        if (aiMat->GetTextureCount(aiTextureType_SPECULAR) > 0) {
         
            aiMat->GetTexture(aiTextureType_SPECULAR, 0, &str);
            std::string textureName = sanitizeTextureName(str);
            material->specularProperty.texture = loadTexture(textureName.c_str());
        }

        // Emissive renk ve tekstürü al       
        if (aiMat->GetTextureCount(aiTextureType_EMISSIVE) > 0) {
           
            aiMat->GetTexture(aiTextureType_EMISSIVE, 0, &str);
            std::string textureName = sanitizeTextureName(str);
            material->emissionProperty.texture = loadTexture(textureName.c_str());
        }
        // Clearcoat Factor
        float clearcoatFactor = 0.0f;  // Default is no clearcoat
        float clearcoatRoughness = 0.0f;  // Default smooth      
        aiMat->Get(AI_MATKEY_CLEARCOAT_ROUGHNESS_FACTOR, clearcoatRoughness);
        aiMat->Get(AI_MATKEY_CLEARCOAT_FACTOR, clearcoatFactor);
        material->setClearcoat(clearcoatFactor, clearcoatRoughness);
        
      
        // Anisotropic Properties (If supported)
        float anisotropy = 0.0f;
        if (aiMat->Get(AI_MATKEY_ANISOTROPY_FACTOR, anisotropy) == AI_SUCCESS) {
            aiColor3D anisotropyDir(0.0f, 0.0f, 0.0f);
            aiMat->Get(AI_MATKEY_ANISOTROPY_FACTOR, anisotropyDir);
            material->setAnisotropic(anisotropy, Vec3SIMD(anisotropyDir.r, anisotropyDir.g, anisotropyDir.b));
        }
           
        if (aiMat->GetTextureCount(aiTextureType_DIFFUSE) > 0) {

            aiMat->GetTexture(aiTextureType_DIFFUSE, 0, &str);
            std::string textureName = sanitizeTextureName(str);
            std::shared_ptr<Texture> diffuseTexture = loadTexture(textureName.c_str());
            material->albedoProperty.texture = diffuseTexture;
            // Diffuse texture'ýn alfa kanalýný opacity olarak kullan
            material->opacityProperty.texture = diffuseTexture;
            material->setOpacityTexture(diffuseTexture, 1);
        }

        // Ayrý bir opacity texture varsa, onu da yükle
        if (aiMat->GetTextureCount(aiTextureType_OPACITY) > 0) {

            aiMat->GetTexture(aiTextureType_OPACITY, 0, &str);
            std::string textureName = sanitizeTextureName(str);
            std::shared_ptr<Texture> opacityTexture = loadTexture(textureName.c_str());
            material->opacityProperty.texture = opacityTexture;
            material->setOpacityTexture(opacityTexture, 1);
        }

        // Opacity (transparency)
        float opacity = 1.0f;
        aiMat->Get(AI_MATKEY_OPACITY, opacity);
        material->opacityProperty = MaterialProperty(opacity);
     
        aiString materialName;
        aiMat->Get(AI_MATKEY_NAME, materialName);
        // Metallic

        std::string materialNameStr = materialName.C_Str();
        std::transform(materialNameStr.begin(), materialNameStr.end(), materialNameStr.begin(), ::tolower);
        if ((materialNameStr.find("glass") != std::string::npos || materialNameStr.find("dielectric") != std::string::npos) ) {
            // Materyal cam olarak tanýmlanmýþ
            auto dielectricMaterial = std::make_shared<Dielectric>(
                1.52,
                Vec3SIMD(0.95, 0.95, 1.0),  // Albedo (color)
                2.0,            // Caustic intensity
                0.3,            // Thickness
                0.1,          // Tint (very slight color tint)
                0.001f         // Roughness
            );
            // Mevcut materyal özelliklerini ata
            dielectricMaterial->albedoProperty = material->albedoProperty;
            dielectricMaterial->roughnessProperty = material->roughnessProperty;
            dielectricMaterial->metallicProperty = material->metallicProperty;
            dielectricMaterial->normalProperty = material->normalProperty;
            dielectricMaterial->opacityProperty = material->opacityProperty;
          
            return dielectricMaterial;
        }

        if ((materialNameStr.find("volume") != std::string::npos || materialNameStr.find("volumetric") != std::string::npos)) {
            // Volumetrik materyal oluþturma
            Vec3SIMD albedo(1.0, 1.0, 1.0); // Duman/sis için renk
            float density = 0.5f;        // Yoðunluk
            float scattering_factor = 0.5f; // Iþýðýn saçýlma oraný
            float absorption_probability = 0.3f; // %30 absorpsiyon
            Vec3SIMD emission = albedo*density;
            auto volumetric_material = std::make_shared<Volumetric>(albedo, density, scattering_factor, absorption_probability, emission);
            volumetric_material->albedoProperty = material->albedoProperty;
          
            return volumetric_material;
        }
        //material->setClearcoat(1, 0);
        return material;
    }
    static std::shared_ptr<Texture> loadTexture(const std::string& filepath) {
        // Base directory where textures are stored

        // Combine the base directory with the given file path
        std::string fullPath = baseDirectory + filepath;

        // Load texture using the full path
        return std::make_shared<Texture>(fullPath);  // Assuming Texture class can handle file loading
    }


    //static std::shared_ptr<Mesh> processMesh(aiMesh* mesh, const aiScene* scene, const std::shared_ptr<Material>& material) {
    //    // Convert Assimp mesh data to Mesh format
    //    std::vector<Vec3SIMD> vertices;
    //    std::vector<Vec3SIMD> normals;
    //    std::vector<Vec2> texCoords;
    //    std::vector<Mesh::Face> faces;

    //    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
    //        vertices.emplace_back(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
    //        normals.emplace_back(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
    //        if (mesh->HasTextureCoords(0)) {
    //            texCoords.emplace_back(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
    //        }
    //        else {
    //            texCoords.emplace_back(0.0f, 0.0f);
    //        }
    //    }

    //    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
    //        aiFace face = mesh->mFaces[i];
    //        Mesh::Face newFace;
    //        for (unsigned int j = 0; j < face.mNumIndices; j++) {
    //            newFace.vertexIndices.push_back(face.mIndices[j]);
    //            newFace.normalIndices.push_back(face.mIndices[j]);
    //            newFace.texCoordIndices.push_back(face.mIndices[j]);
    //        }
    //        faces.push_back(newFace);
    //    }

    //    return std::make_shared<Mesh>(vertices, normals, texCoords, faces, material);
    //}
};
std::vector<std::shared_ptr<Light>> AssimpLoader::lights;