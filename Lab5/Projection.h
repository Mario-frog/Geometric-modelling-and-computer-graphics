#pragma once
// Однорідні координати та матриці перетворень:
//     [x' y' z' w'] = [x y z w] * M
#include <cmath>

constexpr double kPi = 3.14159265358979323846;

struct Vec4 {
    double x = 0, y = 0, z = 0, w = 1;
};

struct Mat4 {
    double m[4][4] = {};
    static Mat4 identity() {
        Mat4 r;
        for (int i = 0; i < 4; ++i) r.m[i][i] = 1.0;
        return r;
    }
};

// Множення вектора точки на матрицю перетворень: v' = v * M
inline Vec4 operator*(const Vec4& v, const Mat4& M) {
    const double in[4] = {v.x, v.y, v.z, v.w};
    double out[4] = {0, 0, 0, 0};
    for (int j = 0; j < 4; ++j)
        for (int i = 0; i < 4; ++i)
            out[j] += in[i] * M.m[i][j];
    return {out[0], out[1], out[2], out[3]};
}

// Композиція перетворень: (v * A) * B == v * (A * B)
inline Mat4 operator*(const Mat4& A, const Mat4& B) {
    Mat4 C;
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            for (int k = 0; k < 4; ++k)
                C.m[i][j] += A.m[i][k] * B.m[k][j];
    return C;
}

//Евклідові перетворення та масштабування
inline Mat4 translation(double dx, double dy, double dz) {
    Mat4 r = Mat4::identity();
    r.m[3][0] = dx; r.m[3][1] = dy; r.m[3][2] = dz;
    return r;
}

inline Mat4 scaling(double s) {
    Mat4 r = Mat4::identity();
    r.m[0][0] = r.m[1][1] = r.m[2][2] = s;
    return r;
}

inline Mat4 rotX(double deg) {
    const double a = deg * kPi / 180.0, c = std::cos(a), s = std::sin(a);
    Mat4 r = Mat4::identity();
    r.m[1][1] = c;  r.m[1][2] = s;
    r.m[2][1] = -s; r.m[2][2] = c;
    return r;
}

inline Mat4 rotY(double deg) {
    const double a = deg * kPi / 180.0, c = std::cos(a), s = std::sin(a);
    Mat4 r = Mat4::identity();
    r.m[0][0] = c;  r.m[0][2] = -s;
    r.m[2][0] = s;  r.m[2][2] = c;
    return r;
}

inline Mat4 rotZ(double deg) {
    const double a = deg * kPi / 180.0, c = std::cos(a), s = std::sin(a);
    Mat4 r = Mat4::identity();
    r.m[0][0] = c;  r.m[0][1] = s;
    r.m[1][0] = -s; r.m[1][1] = c;
    return r;
}

// Метод 1 (варіант 15): ортографічне проекціювання на площини x=0, y=0, z=0 
// Матриці (5.5), (5.6), (5.7) з методички.
inline Mat4 orthoX() {          // площина x = 0
    Mat4 r = Mat4::identity();
    r.m[0][0] = 0.0;
    return r;
}
inline Mat4 orthoY() {          // площина y = 0
    Mat4 r = Mat4::identity();
    r.m[1][1] = 0.0;
    return r;
}
inline Mat4 orthoZ() {          // площина z = 0
    Mat4 r = Mat4::identity();
    r.m[2][2] = 0.0;
    return r;
}
