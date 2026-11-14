#include "Canvas.h"
#include <QPainter>
#include <algorithm>
#include <cmath>

namespace {

constexpr double kRange = 220.0;        // півширина видимої області у вихідних одиницях
constexpr double kTransLimit = 150.0;   // межа, після якої анімований зсув змінює напрям

// Ребра гранованого тіла: 10 вершин нижньої зірки (0..9) та верхньої (10..19)
const std::vector<std::pair<int, int>>& edges() {
    static const std::vector<std::pair<int, int>> e = [] {
        std::vector<std::pair<int, int>> r;
        for (int i = 0; i < 10; ++i) {
            const int j = (i + 1) % 10;
            r.push_back({i, j});            // нижній контур
            r.push_back({10 + i, 10 + j});  // верхній контур
            r.push_back({i, 10 + i});       // вертикальні ребра
        }
        return r;
    }();
    return e;
}

} // namespace

Canvas::Canvas(QWidget* parent) : QWidget(parent) {
    setMinimumSize(820, 680);
    connect(&timer_, &QTimer::timeout, this, [this] { tick(); });
}

void Canvas::setView(int index) { view_ = index; update(); }
void Canvas::setRadius(double r) { radius_ = r; update(); }
void Canvas::setHeight(double h) { height_ = h; update(); }
void Canvas::setInner(double pct) { inner_ = pct; update(); }
void Canvas::setScale(double pct) { scale_ = pct; update(); }

void Canvas::applyTransform(const std::array<double, 3>& pos, const std::array<double, 3>& rotDeg) {
    if (animating_) return;
    pos_ = pos;
    rot_ = rotDeg;
    update();
}

void Canvas::resetAll() {
    if (animating_) return;
    radius_ = 60; height_ = 120; inner_ = 40; scale_ = 100;
    pos_ = {0, 0, 0};
    rot_ = {0, 0, 0};
    update();
}

void Canvas::setAnimation(const std::array<double, 3>& dPos, const std::array<double, 3>& dRotDeg,
                          double hMin, double hMax) {
    dPos_ = dPos;
    dRot_ = dRotDeg;
    hMin_ = std::min(hMin, hMax);
    hMax_ = std::max(hMin, hMax);
}

bool Canvas::toggleAnimation() {
    animating_ = !animating_;
    if (animating_) {
        savedPos_ = pos_;
        savedRot_ = rot_;
        sign_ = {1, 1, 1};
        animH_ = hMin_;
        hDir_ = 1;
        useAnimH_ = true;
        timer_.start(30);
    } else {
        timer_.stop();
        pos_ = savedPos_;      // після зупинки повертаємось у стан до анімації
        rot_ = savedRot_;
        useAnimH_ = false;
        update();
    }
    return animating_;
}

void Canvas::tick() {
    for (int i = 0; i < 3; ++i) {
        pos_[i] += dPos_[i] * sign_[i];
        if (std::abs(pos_[i]) > kTransLimit) {
            sign_[i] = -sign_[i];
            pos_[i] = std::clamp(pos_[i], -kTransLimit, kTransLimit);
        }
        rot_[i] = std::fmod(rot_[i] + dRot_[i], 360.0);
    }
    const double step = std::max(0.5, (hMax_ - hMin_) / 60.0);
    animH_ += step * hDir_;
    if (animH_ >= hMax_) { animH_ = hMax_; hDir_ = -1; }
    if (animH_ <= hMin_) { animH_ = hMin_; hDir_ = 1; }
    update();
}

// Грановане тіло: п'ятикутна зірка-призма. Локальний центр (0,0,0) - "додаткова точка
// всередині фігури", навколо якої виконується обертання.
std::vector<Vec4> Canvas::bodyVertices(double H) const {
    std::vector<Vec4> v;
    const double rOut = radius_;
    const double rIn = radius_ * inner_ / 100.0;
    for (int level = 0; level < 2; ++level) {
        const double y = (level == 0) ? -H / 2 : H / 2;
        for (int i = 0; i < 10; ++i) {
            const double a = (i * 36.0 - 90.0) * kPi / 180.0;
            const double r = (i % 2 == 0) ? rOut : rIn;
            v.push_back({r * std::cos(a), y, r * std::sin(a), 1.0});
        }
    }
    return v;
}

// Масштабування -> обертання навколо власного центру -> зсув у просторі
Mat4 Canvas::modelMatrix() const {
    return scaling(scale_ / 100.0)
         * rotX(rot_[0]) * rotY(rot_[1]) * rotZ(rot_[2])
         * translation(pos_[0], pos_[1], pos_[2]);
}

void Canvas::drawView(QPainter& p, const QRectF& vp, int plane, const QString& title) const {
    p.save();
    p.setClipRect(vp);
    p.setPen(QPen(QColor(190, 190, 190), 1));
    p.setBrush(Qt::NoBrush);
    p.drawRect(vp.adjusted(0.5, 0.5, -0.5, -0.5));

    // Матриця проекціювання відповідно до варіанту (метод 1)
    const Mat4 P = (plane == 0) ? orthoX() : (plane == 1) ? orthoY() : orthoZ();

    const double k = std::min(vp.width(), vp.height()) / (2.0 * kRange);
    const QPointF c = vp.center();
    // Відображення вже спроекційованої точки на екран
    auto toScreen = [&](const Vec4& q) {
        switch (plane) {
        case 0:  return QPointF(c.x() + q.z * k, c.y() - q.y * k);   // x=0: горизонталь Z, вертикаль Y
        case 1:  return QPointF(c.x() + q.x * k, c.y() + q.z * k);   // y=0: горизонталь X, вертикаль Z (вниз)
        default: return QPointF(c.x() + q.x * k, c.y() - q.y * k);   // z=0: горизонталь X, вертикаль Y
        }
    };

    // Осі координат (також множимо на матрицю проекції)
    const double L = kRange * 0.9;
    const Vec4 O = Vec4{0, 0, 0, 1} * P;
    const std::pair<Vec4, QColor> axes[3] = {
        {Vec4{L, 0, 0, 1} * P, QColor(255, 0, 0)},
        {Vec4{0, L, 0, 1} * P, QColor(0, 200, 0)},
        {Vec4{0, 0, L, 1} * P, QColor(0, 0, 255)},
    };
    for (const auto& a : axes) {
        p.setPen(QPen(a.second, 2));
        p.drawLine(toScreen(O), toScreen(a.first));
    }

    // Грановане тіло: вершини * (модельна матриця) * (матриця проекції)
    const Mat4 full = modelMatrix() * P;
    std::vector<QPointF> pts;
    for (const Vec4& v : bodyVertices(currentHeight()))
        pts.push_back(toScreen(v * full));
    p.setPen(QPen(Qt::black, 1.6));
    for (const auto& e : edges())
        p.drawLine(pts[e.first], pts[e.second]);

    p.setPen(QColor(60, 60, 60));
    p.drawText(vp.adjusted(8, 6, -8, -6), Qt::AlignLeft | Qt::AlignTop, title);
    p.restore();
}

void Canvas::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), Qt::white);

    const QString titles[3] = {
        "Площина x = 0 (вигляд збоку)",
        "Площина y = 0 (вигляд зверху)",
        "Площина z = 0 (вигляд спереду)"};

    if (view_ >= 0 && view_ < 3) {
        drawView(p, QRectF(rect()).adjusted(2, 2, -2, -2), view_, titles[view_]);
        return;
    }

    const double w = width() / 2.0, h = height() / 2.0;
    drawView(p, QRectF(0, 0, w, h), 2, titles[2]);
    drawView(p, QRectF(w, 0, w, h), 0, titles[0]);
    drawView(p, QRectF(0, h, w, h), 1, titles[1]);

    p.setPen(Qt::black);
    p.drawText(QRectF(w + 12, h + 12, w - 24, h - 24), Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap,
               "Ортографічне проекціювання (метод 1, варіант 15)\n"
               "[x' y' z' 1] = [x y z 1] · M\n\n"
               "M(x=0) = diag(0, 1, 1, 1)\n"
               "M(y=0) = diag(1, 0, 1, 1)\n"
               "M(z=0) = diag(1, 1, 0, 1)\n\n"
               "Осі: X - червона, Y - зелена, Z - синя");
}
