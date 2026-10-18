// Перевірка математичного ядра без графіки.
#include <cstdio>
#include <cstdlib>
#include "Curve.hpp"
#include "Transform.hpp"

static int fails = 0;
static void check(bool ok, const char* name) {
    std::printf("[%s] %s\n", ok ? " OK " : "FAIL", name);
    if (!ok) ++fails;
}

int main() {
    using namespace hc;
    Params p; // R = 5, r = 1

    // 1. Замкненість: точка на t = 0 і t = 2*pi збігається
    Vec2 a = point(0, p), b = point(2 * PI, p);
    check(std::hypot(a.x - b.x, a.y - b.y) < 1e-9, "крива замкнена за 2*pi (R/r = 5)");

    // 2. Похідні проти скінченних різниць
    const double t0 = 0.7, h = 1e-6;
    Vec2 n1 = point(t0 + h, p), n0 = point(t0 - h, p), d = d1(t0, p);
    check(std::fabs((n1.x - n0.x) / (2 * h) - d.x) < 1e-5 &&
          std::fabs((n1.y - n0.y) / (2 * h) - d.y) < 1e-5, "x'(t), y'(t)");
    Vec2 e1 = d1(t0 + h, p), e0 = d1(t0 - h, p), dd = d2(t0, p);
    check(std::fabs((e1.x - e0.x) / (2 * h) - dd.x) < 1e-5 &&
          std::fabs((e1.y - e0.y) / (2 * h) - dd.y) < 1e-5, "x''(t), y''(t)");

    // 3. Довжина дуги = 8(R - r) = 32, площа = pi r^2 (n-1)(n-2) = 12 pi
    std::printf("L = %.8f (точно %.8f)\n", arcLength(p), arcLengthExact(p));
    std::printf("S = %.8f (точно %.8f)\n", area(p), areaExact(p));
    check(std::fabs(arcLength(p) - 32.0) < 1e-5, "довжина дуги = 32");
    check(std::fabs(area(p) - 12 * PI) < 1e-4, "площа = 12*pi");

    // 4. Інші цілі відношення
    Params q{6, 1}; // R/r = 6
    check(std::fabs(arcLength(q) - 40.0) < 1e-5, "R/r = 6: L = 40");
    check(std::fabs(area(q) - areaExact(q)) < 1e-4, "R/r = 6: площа");
    Params w{4, 1}; // астроїда
    std::printf("астроїда R=4: L = %.6f (6*R = 24 -> очікуємо 24), S = %.6f (3/8 pi R^2 = %.6f)\n",
                arcLength(w), area(w), 3.0 / 8.0 * PI * 16);
    check(std::fabs(arcLength(w) - 24.0) < 1e-5, "астроїда: L = 24");
    check(std::fabs(area(w) - 3.0 / 8.0 * PI * 16) < 1e-4, "астроїда: S = 3/8 pi R^2");

    // 5. Звороти
    auto cu = cusps(p);
    check(cu.size() == 5, "5 точок звороту");
    bool allZero = true;
    for (double t : cu) allZero = allZero && speed(t, p) < 1e-9;
    check(allZero, "у зворотах x' = y' = 0");

    // 6. Точок перегину немає
    check(inflections(p).empty(), "точок перегину немає");

    // 7. Радіус кривини: rho = 4 r (R-r)/(R-2r) |sin(R t/(2r))|, для R=5,r=1: (16/3)|sin(5t/2)|
    double t1 = 0.3;
    double rho = 4.0 * p.r * (p.R - p.r) / (p.R - 2 * p.r) * std::fabs(std::sin(p.R * t1 / (2 * p.r)));
    std::printf("rho(0.3) = %.8f, формула = %.8f\n", curvatureRadius(t1, p), rho);
    check(std::fabs(curvatureRadius(t1, p) - rho) < 1e-8, "радіус кривини збігається з аналітичною формулою");

    // 8. Дотична перпендикулярна нормалі
    LineEq T = tangentEq(t1, p), N = normalEq(t1, p);
    check(std::fabs(T.A * N.A + T.B * N.B) < 1e-9, "дотична ⟂ нормалі");
    Vec2 P = point(t1, p);
    check(std::fabs(T.A * P.x + T.B * P.y + T.C) < 1e-9 &&
          std::fabs(N.A * P.x + N.B * P.y + N.C) < 1e-9, "обидві прямі проходять через точку кривої");

    // 9. Матриці: обертання навколо точки лишає її на місці та зберігає відстані
    Mat3 M = Mat3::rotationAbout(0.9, 2.0, -1.0);
    Vec2 c = M.apply({2.0, -1.0});
    check(std::hypot(c.x - 2.0, c.y + 1.0) < 1e-12, "центр обертання нерухомий");
    Vec2 u = M.apply({0, 0}), v = M.apply({3, 4});
    check(std::fabs(std::hypot(u.x - v.x, u.y - v.y) - 5.0) < 1e-12, "відстані зберігаються");
    Vec2 s = Mat3::translation(1.5, -2).apply({1, 1});
    check(std::fabs(s.x - 2.5) < 1e-12 && std::fabs(s.y + 1.0) < 1e-12, "зсув");

    // 10. Незамкнений випадок
    bool closed = true; Params z{5, 1.3};
    period(z, &closed);
    check(true, "незамкнений випадок не падає");
    std::printf("R/r = %.4f -> замкнена: %s, L = %.4f\n", z.R / z.r, closed ? "так" : "ні", arcLength(z));

    // 11. Нецілий раціональний випадок R/r = 5/2 (період 4*pi)
    Params f{5, 2}; bool cl = false; double T2 = period(f, &cl);
    Vec2 f0 = point(0, f), f1 = point(T2, f);
    check(cl && std::hypot(f0.x - f1.x, f0.y - f1.y) < 1e-9, "R/r = 5/2: замкнена за 4*pi");

    std::printf(fails ? "\nПровалено: %d\n" : "\nУсі перевірки пройдено\n", fails);
    return fails ? 1 : 0;
}
