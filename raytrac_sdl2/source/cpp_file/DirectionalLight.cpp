#include "DirectionalLight.h"

DirectionalLight::DirectionalLight(const Vec3SIMD& dir, const Vec3SIMD& intens, double radius)
    : disk_radius(radius) {
    direction = dir.normalize();
    intensity = intens;
    
}

Vec3SIMD DirectionalLight::random_point() const {
    // Disk �zerindeki rastgele bir nokta
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis_angle(0, 2 * M_PI);
    std::uniform_real_distribution<> dis_radius(0, disk_radius);

    double angle = dis_angle(gen);
    double r = dis_radius(gen);
    double x = r * cos(angle);
    double y = r * sin(angle);
    double z = 0; // Disk d�zleminde z'nin s�f�r oldu�unu varsay�yoruz

    // Disk merkezini ve y�n� ekleyerek ���k kayna��n�n noktas�n� belirleyin
    return Vec3(direction) * 1000.0 + Vec3(x, y, z);
}

LightType DirectionalLight::type() const {
    return LightType::Directional;
}
