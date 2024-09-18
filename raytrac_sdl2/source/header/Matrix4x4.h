#ifndef MATRIX4X4_H
#define MATRIX4X4_H

#include "Vec3.h"  // Vec3 s�n�f� i�in gerekli ba�l�k dosyas�
#include "Vec3SIMD.h"

class Matrix4x4 {
public:
    double m[4][4];
    Matrix4x4() {
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                m[i][j] = (i == j) ? 1.0f : 0.0f; // Identity matrix
    }
    Matrix4x4(Vec3SIMD tangent, Vec3SIMD bitangent, Vec3SIMD normal);
   // Matrix4x4(); // Varsay�lan yap�c�
    // Vec3 ile Matrix4x4 �arpma operat�r�n� arkada� fonksiyon olarak tan�mlay�n
    friend Vec3 operator*(const Matrix4x4& mat, const Vec3SIMD& vec);
    void identity(); // Birim matris olu�turma
    double minor(int row, int col) const;
    double cofactor(int row, int col) const;
    double determinant() const;
    // Transpose metodu
    Matrix4x4 transpose() const;
    
   
    Matrix4x4 inverse() const;
    // Matris �arp�m� operat�r�
    Matrix4x4 operator*(const Matrix4x4& other) const;

    Vec3SIMD transform_point(const Vec3SIMD& p) const; // Nokta d�n���m�
    Vec3SIMD transform_vector(const Vec3SIMD& v) const; // Vekt�r d�n���m�

    // Statik matris olu�turucular�
    static Matrix4x4 translation(const Vec3SIMD& t);
    static Matrix4x4 scaling(const Vec3SIMD& s);
    static Matrix4x4 rotation_x(double angle);
    // Y ve Z eksenleri i�in benzer rotasyon fonksiyonlar� eklenebilir
    static Matrix4x4 translation(double x, double y, double z);
    static Matrix4x4 scaling(double x, double y, double z);
    static Matrix4x4 rotationX(double angle);
    static Matrix4x4 rotationY(double angle);
    static Matrix4x4 rotationZ(double angle);
};

#endif // MATRIX4X4_H
