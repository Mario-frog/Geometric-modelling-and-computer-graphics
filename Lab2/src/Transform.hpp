#pragma once
#include <cmath>
#include "Curve.hpp"

namespace hc {

struct Mat3 {
    double m[3][3];

    static Mat3 identity() {
        return Mat3{{{1, 0, 0},
                     {0, 1, 0},
                     {0, 0, 1}}};
    }

    // Зсув на вектор (dx, dy):
    //  | 1 0 dx |
    //  | 0 1 dy |
    //  | 0 0 1  |
    static Mat3 translation(double dx, double dy) {
        return Mat3{{{1, 0, dx},
                     {0, 1, dy},
                     {0, 0, 1}}};
    }

    // Обертання навколо початку координат на кут a (проти годинникової стрілки):
    //  | cos a  -sin a  0 |
    //  | sin a   cos a  0 |
    //  |   0       0    1 |
    static Mat3 rotation(double a) {
        const double c = std::cos(a), s = std::sin(a);
        return Mat3{{{c, -s, 0},
                     {s,  c, 0},
                     {0,  0, 1}}};
    }

    // Обертання навколо довільної точки (cx, cy):
    //  M = T(cx,cy) * R(a) * T(-cx,-cy)
    static Mat3 rotationAbout(double a, double cx, double cy) {
        return translation(cx, cy) * rotation(a) * translation(-cx, -cy);
    }

    Mat3 operator*(const Mat3& o) const {
        Mat3 r{};
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j) {
                r.m[i][j] = 0;
                for (int k = 0; k < 3; ++k) r.m[i][j] += m[i][k] * o.m[k][j];
            }
        return r;
    }

    // Застосування до точки (w = 1)
    Vec2 apply(const Vec2& p) const {
        return {m[0][0] * p.x + m[0][1] * p.y + m[0][2],
                m[1][0] * p.x + m[1][1] * p.y + m[1][2]};
    }

    // Застосування до вектора (w = 0, без зсуву) - для напрямку дотичної/нормалі
    Vec2 applyVector(const Vec2& v) const {
        return {m[0][0] * v.x + m[0][1] * v.y,
                m[1][0] * v.x + m[1][1] * v.y};
    }
};

} // namespace hc
