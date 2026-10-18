#pragma once
// Гіпоциклоїда (варіант 15): формули, похідні, дотична, нормаль, асимптоти,
// довжина дуги, площа, кривина, точки звороту та перегину.
#include <algorithm>
#include <cmath>
#include <limits>
#include <string>
#include <vector>

namespace hc {

constexpr double PI = 3.14159265358979323846;

struct Vec2 { double x = 0, y = 0; };

//  параметри
// R - радіус нерухомого кола, r - радіус кола, що котиться, k = (R - r) / r.
struct Params {
    double R = 5.0;   // дефолт: R/r = 5 -> крива з 5 вершинами
    double r = 1.0;
    double k() const { return (R - r) / r; }
    bool valid() const { return r > 0 && R > r; }
};

// формула кривої
// x(t) = (R - r) cos t + r cos(k t)
// y(t) = (R - r) sin t - r sin(k t)
inline Vec2 point(double t, const Params& p) {
    const double k = p.k();
    return {(p.R - p.r) * std::cos(t) + p.r * std::cos(k * t),
            (p.R - p.r) * std::sin(t) - p.r * std::sin(k * t)};
}

// похідні
// x'(t) = -(R - r) [sin t + sin(k t)]
// y'(t) =  (R - r) [cos t - cos(k t)]
inline Vec2 d1(double t, const Params& p) {
    const double k = p.k(), a = p.R - p.r;
    return {-a * (std::sin(t) + std::sin(k * t)),
             a * (std::cos(t) - std::cos(k * t))};
}

// x''(t) = -(R - r) [cos t + k cos(k t)]
// y''(t) =  (R - r) [k sin(k t) - sin t]
inline Vec2 d2(double t, const Params& p) {
    const double k = p.k(), a = p.R - p.r;
    return {-a * (std::cos(t) + k * std::cos(k * t)),
             a * (k * std::sin(k * t) - std::sin(t))};
}

// Кутовий коефіцієнт дотичної dy/dx (inf - вертикальна дотична / вершина)
inline double slope(double t, const Params& p) {
    const Vec2 d = d1(t, p);
    return std::fabs(d.x) > 1e-12 ? d.y / d.x
                                  : std::numeric_limits<double>::infinity();
}

// Швидкість |v(t)| = sqrt(x'^2 + y'^2) = 2 (R - r) |sin(R t / (2 r))|
inline double speed(double t, const Params& p) {
    const Vec2 d = d1(t, p);
    return std::hypot(d.x, d.y);
}

// період / замкненість
// Крива замкнена, якщо R/r = p/q (раціональне); тоді період T = 2*pi*q.
inline double period(const Params& p, bool* closed = nullptr) {
    const double ratio = p.R / p.r;
    for (int q = 1; q <= 100; ++q) {
        const double v = ratio * q;
        if (std::fabs(v - std::round(v)) < 1e-6 * std::max(1.0, v)) {
            if (closed) *closed = true;
            return 2.0 * PI * q;
        }
    }
    if (closed) *closed = false;
    return 2.0 * PI * 20.0; // незамкнена "розетка": малюємо 20 обертів
}

// дотична та нормаль
// Загальне рівняння прямої: A X + B Y + C = 0.
struct LineEq { double A, B, C; };

// Дотична: y'(X - x0) - x'(Y - y0) = 0
inline LineEq tangentEq(double t, const Params& p) {
    const Vec2 P = point(t, p), d = d1(t, p);
    return {d.y, -d.x, -(d.y * P.x - d.x * P.y)};
}

// Нормаль: x'(X - x0) + y'(Y - y0) = 0
inline LineEq normalEq(double t, const Params& p) {
    const Vec2 P = point(t, p), d = d1(t, p);
    return {d.x, d.y, -(d.x * P.x + d.y * P.y)};
}

struct Segment { Vec2 a, b; };

// Відрізок дотичної (довжина 2*len); false - у вершині дотична не визначена
inline bool tangentSegment(double t, const Params& p, double len, Segment& out) {
    const Vec2 P = point(t, p), d = d1(t, p);
    const double n = std::hypot(d.x, d.y);
    if (n < 1e-9) return false;
    const double ux = d.x / n, uy = d.y / n;
    out = {{P.x - len * ux, P.y - len * uy}, {P.x + len * ux, P.y + len * uy}};
    return true;
}

// Відрізок нормалі; одиничний вектор нормалі n = (-y', x') / |v|
inline bool normalSegment(double t, const Params& p, double len, Segment& out) {
    const Vec2 P = point(t, p), d = d1(t, p);
    const double n = std::hypot(d.x, d.y);
    if (n < 1e-9) return false;
    const double nx = -d.y / n, ny = d.x / n;
    out = {{P.x - len * nx, P.y - len * ny}, {P.x + len * nx, P.y + len * ny}};
    return true;
}

//  асимптоти
// Гіпоциклоїда - обмежена замкнена крива (|x|,|y| <= R): нескінченних віток
// немає, отже асимптот не існує.
inline const char* asymptoteInfo() { return "Асимптот не існує (крива обмежена)"; }

//  кривина
// cross = x' y'' - y' x''
inline double cross(double t, const Params& p) {
    const Vec2 a = d1(t, p), b = d2(t, p);
    return a.x * b.y - a.y * b.x;
}

// Знакова кривина k(t) = (x'y'' - y'x'') / (x'^2 + y'^2)^(3/2)
inline double curvature(double t, const Params& p) {
    const Vec2 d = d1(t, p);
    const double den = std::pow(d.x * d.x + d.y * d.y, 1.5);
    if (den < 1e-9) return std::numeric_limits<double>::infinity();
    return cross(t, p) / den;
}

// Радіус кривини rho = 1/|k| = (x'^2+y'^2)^(3/2) / |x'y'' - y'x''|
// (у вершині rho = 0, кривина -> inf)
inline double curvatureRadius(double t, const Params& p) {
    const Vec2 d = d1(t, p);
    const double den = std::pow(d.x * d.x + d.y * d.y, 1.5);
    const double c = std::fabs(cross(t, p));
    if (den < 1e-9) return 0.0;
    if (c < 1e-12) return std::numeric_limits<double>::infinity();
    return den / c;
}

// Центр кривини: C = P + rho * n
inline bool curvatureCenter(double t, const Params& p, Vec2& c) {
    const Vec2 P = point(t, p), d = d1(t, p);
    const double v2 = d.x * d.x + d.y * d.y;
    const double cr = cross(t, p);
    if (v2 < 1e-9 || std::fabs(cr) < 1e-12) return false;
    c = {P.x - d.y * v2 / cr, P.y + d.x * v2 / cr};
    return true;
}

//  інтегрування (Сімпсон)
template <class F>
inline double simpson(F f, double a, double b, int n) {
    if (n % 2) ++n;
    const double h = (b - a) / n;
    double s = f(a) + f(b);
    for (int i = 1; i < n; ++i) s += f(a + i * h) * (i % 2 ? 4.0 : 2.0);
    return s * h / 3.0;
}

inline int samplesFor(const Params& p) {
    const double T = period(p);
    return 4000 * (int)std::ceil(T / (2 * PI));
}

// Довжина дуги L = integral sqrt(x'^2 + y'^2) dt по періоду
inline double arcLength(const Params& p) {
    return simpson([&](double t) { return speed(t, p); }, 0, period(p), samplesFor(p));
}

// Довжина дуги від t1 до t2
inline double arcLength(const Params& p, double t1, double t2) {
    return simpson([&](double t) { return speed(t, p); }, t1, t2, 2000);
}

// Площа за формулою Гріна: S = 1/2 |integral (x y' - y x') dt|
inline double area(const Params& p) {
    return std::fabs(0.5 * simpson([&](double t) {
        const Vec2 P = point(t, p), d = d1(t, p);
        return P.x * d.y - P.y * d.x;
    }, 0, period(p), samplesFor(p)));
}

// Закриті формули для R/r = n (ціле):  L = 8 (R - r),  S = pi r^2 (n-1)(n-2)
inline bool isIntegerRatio(const Params& p, int* n = nullptr) {
    const double q = p.R / p.r;
    if (std::fabs(q - std::round(q)) < 1e-9) {
        if (n) *n = (int)std::round(q);
        return true;
    }
    return false;
}
inline double arcLengthExact(const Params& p) { return 8.0 * (p.R - p.r); }
inline double areaExact(const Params& p) {
    const double n = p.R / p.r;
    return PI * p.r * p.r * (n - 1) * (n - 2);
}

//  точки звороту та перегину
// Звороти: x' = y' = 0  <=>  t = 2*pi*m*r/R
inline std::vector<double> cusps(const Params& p) {
    std::vector<double> v;
    const double T = period(p), step = 2.0 * PI * p.r / p.R;
    for (double t = 0; t < T - 1e-9; t += step) v.push_back(t);
    return v;
}

// Точки перегину: зміна знаку cross(t) поза точками звороту (бісекція)
inline std::vector<double> inflections(const Params& p) {
    std::vector<double> res;
    const double T = period(p);
    const int N = samplesFor(p);
    const double h = T / N, tol = 1e-3 * (p.R - p.r);
    auto ok = [&](double t) { return speed(t, p) > tol; };
    double tp = 0, cp = cross(tp, p);
    for (int i = 1; i <= N; ++i) {
        const double t = i * h, c = cross(t, p);
        if (ok(t) && ok(tp) && cp * c < 0) {
            double a = tp, b = t;
            for (int it = 0; it < 60; ++it) {
                const double m = 0.5 * (a + b);
                if (cross(a, p) * cross(m, p) <= 0) b = m; else a = m;
            }
            res.push_back(0.5 * (a + b));
        }
        tp = t; cp = c;
    }
    return res;
}

} // namespace hc
