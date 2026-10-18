// Лабораторна робота №2. Варіант 15 - Гіпоциклоїда.
#include <SFML/Graphics.hpp>

#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <string>
#include <vector>

#include "Curve.hpp"
#include "Transform.hpp"

using namespace hc;

//  утиліти
static std::string fmt(const char* f, ...) {
    char buf[512];
    va_list ap;
    va_start(ap, f);
    std::vsnprintf(buf, sizeof buf, f, ap);
    va_end(ap);
    return buf;
}

static sf::String U(const std::string& s) { return sf::String::fromUtf8(s.begin(), s.end()); }

static double round2(double v) { return std::round(v * 100.0) / 100.0; }

static std::string lineStr(double A, double B, double C) {
    return fmt("%.3f x %+.3f y %+.3f = 0", A, B, C);
}

// Обчислення "важких" характеристик (кешуються до зміни параметрів)
struct Stats {
    double L = 0, S = 0, T = 0;
    int cusps = 0, infl = 0;
    bool closed = true, integer = false;
    int n = 0;
};

static Stats computeStats(const Params& p) {
    Stats s;
    s.T = period(p, &s.closed);
    s.L = arcLength(p);
    s.S = area(p);
    s.cusps = (int)cusps(p).size();
    s.infl = (int)inflections(p).size();
    s.integer = isIntegerRatio(p, &s.n);
    return s;
}

//  вікно
static const int W = 1200, H = 800, PANEL = 390;

struct View {
    double scale = 40.0;                          // пікселів на одиницю
    double ox = PANEL + (W - PANEL) / 2.0, oy = H / 2.0;
    sf::Vector2f toScreen(const Vec2& p) const { return {(float)(ox + p.x * scale), (float)(oy - p.y * scale)}; }
    Vec2 toWorld(float sx, float sy) const { return {(sx - ox) / scale, (oy - sy) / scale}; }
};

static bool loadFont(sf::Font& f) {
    const char* paths[] = {
#ifdef ASSET_DIR
        ASSET_DIR "/DejaVuSans.ttf",
#endif
        "assets/DejaVuSans.ttf", "../assets/DejaVuSans.ttf", "DejaVuSans.ttf",
        "C:/Windows/Fonts/arial.ttf", "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/System/Library/Fonts/Supplemental/Arial.ttf"};
    for (const char* p : paths)
        if (f.loadFromFile(p)) return true;
    return false;
}

//  малювання
static void drawGrid(sf::RenderWindow& win, const View& v) {
    const Vec2 a = v.toWorld(PANEL, H), b = v.toWorld(W, 0);
    sf::VertexArray minor(sf::Lines), major(sf::Lines);
    const double step = v.scale >= 25 ? 0.5 : (v.scale >= 10 ? 1.0 : 5.0);
    const sf::Color cMinor(235, 235, 235), cMajor(210, 210, 210);
    for (double x = std::floor(a.x / step) * step; x <= b.x; x += step) {
        const bool isMajor = std::fabs(std::fmod(std::fabs(x), 1.0)) < 1e-9 || std::fabs(std::fmod(std::fabs(x), 1.0) - 1.0) < 1e-9;
        sf::VertexArray& va = isMajor ? major : minor;
        const sf::Color c = isMajor ? cMajor : cMinor;
        va.append({v.toScreen({x, a.y}), c});
        va.append({v.toScreen({x, b.y}), c});
    }
    for (double y = std::floor(a.y / step) * step; y <= b.y; y += step) {
        const bool isMajor = std::fabs(std::fmod(std::fabs(y), 1.0)) < 1e-9 || std::fabs(std::fmod(std::fabs(y), 1.0) - 1.0) < 1e-9;
        sf::VertexArray& va = isMajor ? major : minor;
        const sf::Color c = isMajor ? cMajor : cMinor;
        va.append({v.toScreen({a.x, y}), c});
        va.append({v.toScreen({b.x, y}), c});
    }
    win.draw(minor);
    win.draw(major);
}

static void drawAxes(sf::RenderWindow& win, const View& v) {
    sf::Vertex xa[] = {{{(float)PANEL, (float)v.oy}, sf::Color::Red}, {{(float)W, (float)v.oy}, sf::Color::Red}};
    sf::Vertex ya[] = {{{(float)v.ox, 0.f}, sf::Color(0, 200, 0)}, {{(float)v.ox, (float)H}, sf::Color(0, 200, 0)}};
    win.draw(xa, 2, sf::Lines);
    win.draw(ya, 2, sf::Lines);
}

static void drawSegment(sf::RenderWindow& win, const View& v, const Mat3& M, const Segment& s, sf::Color c) {
    sf::Vertex l[] = {{v.toScreen(M.apply(s.a)), c}, {v.toScreen(M.apply(s.b)), c}};
    win.draw(l, 2, sf::Lines);
}

static void drawDot(sf::RenderWindow& win, sf::Vector2f p, float r, sf::Color c) {
    sf::CircleShape d(r);
    d.setOrigin(r, r);
    d.setPosition(p);
    d.setFillColor(c);
    win.draw(d);
}

static void drawCircle(sf::RenderWindow& win, sf::Vector2f c, float r, sf::Color col) {
    sf::CircleShape d(r, 120);
    d.setOrigin(r, r);
    d.setPosition(c);
    d.setFillColor(sf::Color::Transparent);
    d.setOutlineColor(col);
    d.setOutlineThickness(1.5f);
    win.draw(d);
}

//  main
int main(int argc, char** argv) {
    std::string shotFile;
    bool demo = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--shot" && i + 1 < argc) shotFile = argv[++i];
        if (std::string(argv[i]) == "--demo") demo = true;
    }
    sf::RenderWindow win(sf::VideoMode(W, H), U("Лабораторна №2 - Гіпоциклоїда (варіант 15)"),
                         sf::Style::Titlebar | sf::Style::Close, sf::ContextSettings(0, 0, 8));
    win.setFramerateLimit(60);

    sf::Font font;
    const bool haveFont = loadFont(font);
    if (!haveFont) std::fprintf(stderr, "Шрифт не знайдено: текст не буде показано (покладіть assets/DejaVuSans.ttf поруч).\n");

    View view;
    Params P;                       // дефолтні R = 5, r = 1
    Mat3 M = Mat3::identity();      // накопичене евклідове перетворення
    Vec2 pivot{0, 0};               // центр обертання (в екранних світових координатах)
    double t0 = 0.6;                // параметр точки дотику
    double rotDeg = 0;              // накопичений кут
    bool anim = false, grid = true, helpOn = true, circles = false, oscul = false;
    double animTime = 0, rBase = P.r;
    int frame = 0;
    Stats st = computeStats(P);
    bool dirty = false;

    if (demo) {   // службовий режим для скріншота
        circles = true; oscul = true; t0 = 1.2;
        M = Mat3::rotationAbout(20 * PI / 180, 1, 1) * Mat3::translation(1.0, 0.5);
        pivot = {1, 1}; rotDeg = 20;
    }
    int shotFrames = 0;
    sf::Clock clock;
    while (win.isOpen()) {
        const double dt = clock.restart().asSeconds();
        sf::Event e;
        while (win.pollEvent(e)) {
            if (e.type == sf::Event::Closed) win.close();

            if (e.type == sf::Event::MouseWheelScrolled) {
                view.scale *= (e.mouseWheelScroll.delta > 0) ? 1.1 : 1.0 / 1.1;
                view.scale = std::max(5.0, std::min(view.scale, 300.0));
            }

            if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.x > PANEL) {
                const Vec2 w = view.toWorld((float)e.mouseButton.x, (float)e.mouseButton.y);
                if (e.mouseButton.button == sf::Mouse::Right) {
                    pivot = w;           // центр обертання
                } else if (e.mouseButton.button == sf::Mouse::Left) {
                    // вибір найближчої точки кривої
                    const int N = 4000;
                    double best = 1e18, bt = t0;
                    for (int i = 0; i < N; ++i) {
                        const double t = st.T * i / N;
                        const Vec2 q = M.apply(point(t, P));
                        const double d = (q.x - w.x) * (q.x - w.x) + (q.y - w.y) * (q.y - w.y);
                        if (d < best) { best = d; bt = t; }
                    }
                    t0 = bt;
                }
            }

            if (e.type == sf::Event::KeyPressed) {
                const bool shift = e.key.shift;
                const double dp = shift ? 1.0 : 0.1;
                const double mv = shift ? 1.0 : 0.25;
                Params old = P;
                switch (e.key.code) {
                    case sf::Keyboard::Escape: win.close(); break;
                    // --- параметри кривої ---
                    case sf::Keyboard::Q: P.R = round2(P.R + dp); break;
                    case sf::Keyboard::A: P.R = round2(P.R - dp); break;
                    case sf::Keyboard::W: P.r = round2(P.r + dp); break;
                    case sf::Keyboard::S: P.r = round2(P.r - dp); break;
                    // --- точка дотичної ---
                    case sf::Keyboard::Comma:  t0 -= 0.05; break;
                    case sf::Keyboard::Period: t0 += 0.05; break;
                    // --- зсув ---
                    case sf::Keyboard::Left:  M = Mat3::translation(-mv, 0) * M; pivot.x -= mv; break;
                    case sf::Keyboard::Right: M = Mat3::translation(mv, 0) * M;  pivot.x += mv; break;
                    case sf::Keyboard::Up:    M = Mat3::translation(0, mv) * M;  pivot.y += mv; break;
                    case sf::Keyboard::Down:  M = Mat3::translation(0, -mv) * M; pivot.y -= mv; break;
                    // --- обертання навколо pivot ---
                    case sf::Keyboard::Z: M = Mat3::rotationAbout(5 * PI / 180, pivot.x, pivot.y) * M; rotDeg += 5; break;
                    case sf::Keyboard::X: M = Mat3::rotationAbout(-5 * PI / 180, pivot.x, pivot.y) * M; rotDeg -= 5; break;
                    case sf::Keyboard::C: pivot = M.apply({0, 0}); break;   // pivot -> центр кривої
                    case sf::Keyboard::Backspace: M = Mat3::identity(); pivot = {0, 0}; rotDeg = 0; break;
                    // --- режими ---
                    case sf::Keyboard::Space:
                        anim = !anim;
                        if (anim) { rBase = P.r; animTime = 0; }
                        else { P.r = rBase; dirty = true; }
                        break;
                    case sf::Keyboard::G: grid = !grid; break;
                    case sf::Keyboard::H: helpOn = !helpOn; break;
                    case sf::Keyboard::O: circles = !circles; break;
                    case sf::Keyboard::K: oscul = !oscul; break;
                    case sf::Keyboard::Enter: P = Params(); anim = false; rBase = P.r; dirty = true; break;
                    default: break;
                }
                if (!P.valid() || P.R > 20 || P.r < 0.1) P = old;       // захист від некоректних значень
                if (P.R != old.R || P.r != old.r) { if (!anim) rBase = P.r; dirty = true; }
            }
        }

        //  анімація
        // Автоматично змінюються: радіус r (R/r "пливе") і параметр t0 (точка дотичної)
        if (anim) {
            animTime += dt;
            P.r = std::max(0.3, rBase * (1.0 + 0.35 * std::sin(0.7 * animTime)));
            if (P.r >= P.R) P.r = P.R - 0.1;
            t0 += dt * 0.8;
            if (++frame % 6 == 0) dirty = true;
        }
        if (dirty) { st = computeStats(P); dirty = false; }
        if (t0 > st.T) t0 -= st.T;
        if (t0 < 0) t0 += st.T;

        
        win.clear(sf::Color::White);
        if (grid) drawGrid(win, view);
        drawAxes(win, view);

        // Крива
        const double Tdraw = (anim && !st.closed) ? 2 * PI : st.T;
        const int N = std::min(8000, 1500 * (int)std::ceil(Tdraw / (2 * PI)));
        sf::VertexArray curve(sf::LineStrip, N + 1);
        for (int i = 0; i <= N; ++i) {
            curve[i].position = view.toScreen(M.apply(point(Tdraw * i / N, P)));
            curve[i].color = sf::Color::Black;
        }
        win.draw(curve);

        // Вершини (точки звороту)
        for (double t : cusps(P)) drawDot(win, view.toScreen(M.apply(point(t, P))), 3.5f, sf::Color(30, 80, 220));

        // Кола, що "котяться"
        if (circles) {
            drawCircle(win, view.toScreen(M.apply({0, 0})), (float)(P.R * view.scale), sf::Color(150, 150, 150));
            const Vec2 c = {(P.R - P.r) * std::cos(t0), (P.R - P.r) * std::sin(t0)};
            drawCircle(win, view.toScreen(M.apply(c)), (float)(P.r * view.scale), sf::Color(220, 140, 40));
            sf::Vertex rad[] = {{view.toScreen(M.apply(c)), sf::Color(220, 140, 40)},
                                {view.toScreen(M.apply(point(t0, P))), sf::Color(220, 140, 40)}};
            win.draw(rad, 2, sf::Lines);
        }

        // Дотична та нормаль в точці t0
        const Vec2 Pm = point(t0, P);          // в системі кривої
        const Vec2 Ps = M.apply(Pm);           // після перетворення
        Segment seg;
        const bool hasTan = tangentSegment(t0, P, 6.0, seg);
        if (hasTan) {
            drawSegment(win, view, M, seg, sf::Color(30, 80, 220));
            normalSegment(t0, P, 6.0, seg);
            drawSegment(win, view, M, seg, sf::Color(240, 120, 0));
        }
        // Коло кривини
        Vec2 cc;
        if (oscul && curvatureCenter(t0, P, cc)) {
            drawCircle(win, view.toScreen(M.apply(cc)), (float)(curvatureRadius(t0, P) * view.scale), sf::Color(160, 0, 160));
        }
        drawDot(win, view.toScreen(Ps), 5.f, sf::Color::Red);

        // Центр обертання
        {
            const sf::Vector2f c = view.toScreen(pivot);
            sf::Vertex a[] = {{{c.x - 7, c.y}, sf::Color::Magenta}, {{c.x + 7, c.y}, sf::Color::Magenta},
                              {{c.x, c.y - 7}, sf::Color::Magenta}, {{c.x, c.y + 7}, sf::Color::Magenta}};
            win.draw(a, 4, sf::Lines);
        }

        //панель тексту
        sf::RectangleShape panel({(float)PANEL, (float)H});
        panel.setFillColor(sf::Color(250, 250, 250));
        panel.setOutlineColor(sf::Color(180, 180, 180));
        panel.setOutlineThickness(1.f);
        win.draw(panel);

        if (haveFont) {
            std::string s;
            s += "ГІПОЦИКЛОЇДА (варіант 15)\n";
            s += "x = (R-r)cos t + r cos(kt)\n";
            s += "y = (R-r)sin t - r sin(kt),  k=(R-r)/r\n\n";
            s += fmt("R = %.2f   r = %.2f   R/r = %.3f   k = %.3f\n", P.R, P.r, P.R / P.r, P.k());
            s += fmt("Анімація: %s   Крива замкнена: %s\n\n", anim ? "ТАК" : "ні", st.closed ? "так" : "ні");

            s += fmt("t0 = %.3f рад\n", t0);
            s += fmt("Точка: (%.3f; %.3f)\n", Ps.x, Ps.y);
            if (hasTan) {
                const Vec2 dm = d1(t0, P);
                const Vec2 dv = M.applyVector(dm);             // напрям дотичної після перетворення
                const double A = dv.y, B = -dv.x, C = -(A * Ps.x + B * Ps.y);
                const double An = dv.x, Bn = dv.y, Cn = -(An * Ps.x + Bn * Ps.y);
                s += "Дотична: " + lineStr(A, B, C) + "\n";
                s += "Нормаль: " + lineStr(An, Bn, Cn) + "\n";
                const double rho = curvatureRadius(t0, P);
                s += fmt("Радіус кривини ρ = %.4f\n", rho);
            } else {
                s += "Вершина: дотична не визначена, ρ = 0\n";
            }
            s += fmt("%s\n\n", asymptoteInfo());

            s += "ХАРАКТЕРИСТИКИ:\n";
            s += fmt("Довжина дуги L = %.5f\n", st.L);
            s += fmt("Площа S = %.5f\n", st.S);
            if (st.integer) s += fmt("(формули: L = 8(R-r) = %.5f, S = %.5f)\n", arcLengthExact(P), areaExact(P));
            s += fmt("Точок звороту: %d\n", st.cusps);
            s += fmt("Точок перегину: %d\n\n", st.infl);

            s += "ПЕРЕТВОРЕННЯ (матриця M):\n";
            for (int i = 0; i < 3; ++i) s += fmt("| %7.3f %7.3f %7.3f |\n", M.m[i][0], M.m[i][1], M.m[i][2]);
            s += fmt("Кут: %.0f°  Центр обертання: (%.2f; %.2f)\n\n", rotDeg, pivot.x, pivot.y);

            if (helpOn) {
                s += "КЕРУВАННЯ (H - сховати):\n";
                s += "Q/A  R +/-   W/S  r +/-  (Shift - крок 1)\n";
                s += "Space  анімація    Enter  скинути\n";
                s += "ЛКМ  вибрати точку дотичної  , .  рухати\n";
                s += "ПКМ  центр обертання  C  у центр кривої\n";
                s += "Стрілки  зсув    Z/X  обертання ±5°\n";
                s += "Backspace  скинути перетворення\n";
                s += "G сітка  O кола  K коло кривини  Колесо - масштаб\n";
            }
            sf::Text txt(U(s), font, 14);
            txt.setFillColor(sf::Color::Black);
            txt.setPosition(10, 8);
            win.draw(txt);
        }
        win.display();
        if (!shotFile.empty() && ++shotFrames == 3) {
            sf::Texture tex; tex.create(W, H); tex.update(win);
            tex.copyToImage().saveToFile(shotFile);
            win.close();
        }
    }
    return 0;
}
