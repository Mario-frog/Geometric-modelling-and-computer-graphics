#include "Canvas.h"
#include <QPainter>
#include <QMouseEvent>
#include <QFileDialog>
#include <QtMath>
#include <algorithm>

namespace {


QPointF bezier3(const QPointF& a, const QPointF& b, const QPointF& c, const QPointF& d, double t) {
    const double u = 1.0 - t;
    const double b0 = u * u * u;
    const double b1 = 3 * u * u * t;
    const double b2 = 3 * u * t * t;
    const double b3 = t * t * t;
    return a * b0 + b * b1 + c * b2 + d * b3;
}


constexpr double kScale = 3.5;
const QPointF kOffset(70, 20);
constexpr double kBgWidth = 187;

}

Canvas::Canvas(QWidget* parent) : QWidget(parent) {
    setMinimumSize(820, 680);
    setAttribute(Qt::WA_OpaquePaintEvent);
    resetShape();
    connect(&timer_, &QTimer::timeout, this, [this] {
        phase_ += 0.012 * dir_;
        if (phase_ >= 1.0) { phase_ = 1.0; dir_ = -1; }
        if (phase_ <= 0.0) { phase_ = 0.0; dir_ = 1; }
        s_ = phase_ * phase_ * (3 - 2 * phase_);  // smoothstep
        update();
    });
}


void Canvas::resetShape() {
    if (animating_) toggleAnimation();
    static const QPointF raw[] = {
        {9.2, 99.0},
        {15.6, 88.0},
        {25.4, 57.5},
        {34.8, 42.2},
        {44.8, 32.4},
        {58.2, 24.5},
        {69.5, 21.1},
        {85.2, 19.7},
        {106.8, 1.8},
        {110.8, 0.1},
        {122.3, 0.2},
        {118.9, 5.0},
        {115.8, 19.0},
        {116.0, 25.5},
        {120.8, 35.0},
        {144.4, 58.8},
        {149.7, 66.2},
        {154.4, 75.8},
        {157.7, 87.8},
        {160.0, 117.8},
        {163.4, 125.5},
        {173.7, 137.2},
        {177.6, 147.2},
        {177.6, 153.1},
        {174.5, 156.0},
        {173.3, 159.9},
        {171.6, 160.1},
        {165.5, 152.4},
        {152.5, 145.7},
        {145.2, 148.7},
        {133.0, 149.2},
        {125.8, 154.5},
        {123.9, 154.1},
        {122.1, 151.2},
        {122.0, 145.2},
        {124.7, 138.0},
        {140.7, 123.2},
        {143.0, 117.2},
        {139.6, 108.8},
        {127.1, 90.0},
        {122.5, 85.4},
        {117.2, 82.9},
        {111.0, 82.2},
        {105.2, 87.8},
        {93.5, 91.7},
        {80.2, 91.6},
        {63.5, 84.9},
        {60.5, 84.9},
        {36.8, 93.1},
        {20.2, 107.3},
        {16.0, 108.7},
        {10.3, 107.4}};
    std::vector<QPointF> A;
    for (const auto& r : raw) A.push_back(r * kScale + kOffset);

    const int n = static_cast<int>(A.size());
    pts_.assign(3 * n, QPointF());

    auto norm = [](QPointF v) {
        const double l = std::hypot(v.x(), v.y());
        return l > 1e-9 ? v / l : QPointF(0, 0);
    };
    auto len = [](QPointF v) { return std::hypot(v.x(), v.y()); };

    std::vector<QPointF> dir(n);
    std::vector<bool> corner(n, false);
    for (int i = 0; i < n; ++i) {
        const QPointF in = A[i] - A[(i + n - 1) % n];
        const QPointF out = A[(i + 1) % n] - A[i];
        const QPointF a = norm(in), b = norm(out);
        const double cosang = a.x() * b.x() + a.y() * b.y();
        corner[i] = cosang < 0.5;  // поворот більше ~60 градусів
        dir[i] = norm(a + b);
    }

    for (int i = 0; i < n; ++i) {
        const int j = (i + 1) % n;
        const QPointF seg = A[j] - A[i];
        const double L = len(seg) / 3.0;
        pts_[3 * i] = A[i];
        pts_[3 * i + 1] = corner[i] ? A[i] + seg / 3.0 : A[i] + dir[i] * L;
        pts_[3 * i + 2] = corner[j] ? A[j] - seg / 3.0 : A[j] - dir[j] * L;
    }
    tension_ = 1.0;
    update();
}

void Canvas::setShowPolygon(bool on) { showPolygon_ = on; update(); }
void Canvas::setSmoothJoints(bool on) { smooth_ = on; }

void Canvas::setTension(int percent) {
    if (animating_) return;
    const double k = std::max(0.05, percent / 100.0);
    const double f = k / tension_;
    const int m = static_cast<int>(pts_.size());
    for (int i = 0; i < m / 3; ++i) {
        const QPointF a = pts_[3 * i];
        const QPointF an = pts_[(3 * i + 3) % m];
        pts_[3 * i + 1] = a + (pts_[3 * i + 1] - a) * f;
        pts_[3 * i + 2] = an + (pts_[3 * i + 2] - an) * f;
    }
    tension_ = k;
    update();
}

std::vector<QPointF> Canvas::buildTriangleTarget() const {
    double minX = 1e9, maxX = -1e9, minY = 1e9, maxY = -1e9;
    for (const auto& p : pts_) {
        minX = std::min(minX, p.x()); maxX = std::max(maxX, p.x());
        minY = std::min(minY, p.y()); maxY = std::max(maxY, p.y());
    }
    const QPointF v[3] = {{(minX + maxX) / 2, minY}, {maxX, maxY}, {minX, maxY}};
    double len[3], total = 0;
    for (int i = 0; i < 3; ++i) {
        const QPointF d = v[(i + 1) % 3] - v[i];
        len[i] = std::hypot(d.x(), d.y());
        total += len[i];
    }
    auto pointAt = [&](double f) {
        double dist = f * total;
        for (int i = 0; i < 3; ++i) {
            if (dist <= len[i] || i == 2) {
                const double t = len[i] > 0 ? dist / len[i] : 0;
                return v[i] + (v[(i + 1) % 3] - v[i]) * t;
            }
            dist -= len[i];
        }
        return v[0];
    };
    const int n = static_cast<int>(pts_.size()) / 3;
    std::vector<QPointF> out(pts_.size());
    for (int i = 0; i < n; ++i) {
        out[3 * i]     = pointAt(double(i) / n);
        out[3 * i + 1] = pointAt((i + 1.0 / 3) / n);
        out[3 * i + 2] = pointAt((i + 2.0 / 3) / n);
    }
    return out;
}

void Canvas::toggleAnimation() {
    animating_ = !animating_;
    if (animating_) {
        target_ = buildTriangleTarget();
        phase_ = 0; s_ = 0; dir_ = 1;
        timer_.start(16);
    } else {
        timer_.stop();
        s_ = 0;
        update();
    }
}

std::vector<QPointF> Canvas::currentPoints() const {
    if (!animating_ || target_.size() != pts_.size()) return pts_;
    std::vector<QPointF> r(pts_.size());
    for (size_t i = 0; i < pts_.size(); ++i) r[i] = pts_[i] * (1 - s_) + target_[i] * s_;
    return r;
}


QPointF Canvas::centroid() const {
    QPointF c(0, 0);
    const int n = static_cast<int>(pts_.size()) / 3;
    for (int i = 0; i < n; ++i) c += pts_[3 * i];
    return c / n;
}

void Canvas::applyTransform(const QTransform& t) {
    if (animating_) return;
    for (auto& p : pts_) p = t.map(p);
    update();
}

void Canvas::rotate(double deg) {
    const QPointF c = centroid();
    QTransform t;
    t.translate(c.x(), c.y());
    t.rotate(deg);
    t.translate(-c.x(), -c.y());
    applyTransform(t);
}

void Canvas::flipH() {
    const QPointF c = centroid();
    QTransform t;
    t.translate(c.x(), 0); t.scale(-1, 1); t.translate(-c.x(), 0);
    applyTransform(t);
}

void Canvas::flipV() {
    const QPointF c = centroid();
    QTransform t;
    t.translate(0, c.y()); t.scale(1, -1); t.translate(0, -c.y());
    applyTransform(t);
}

void Canvas::move(double dx, double dy) {
    QTransform t;
    t.translate(dx, dy);
    applyTransform(t);
}

void Canvas::loadBackground() {
    const QString f = QFileDialog::getOpenFileName(this, "Фоновая картинка", {}, "Images (*.png *.jpg *.jpeg *.bmp)");
    if (f.isEmpty()) return;
    bg_.load(f);
    update();
}


void Canvas::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), Qt::white);

    if (!bg_.isNull()) {
        p.setOpacity(0.30);
        const double w = kBgWidth * kScale;
        p.drawPixmap(QRectF(kOffset.x(), kOffset.y(), w, w * bg_.height() / bg_.width()), bg_, bg_.rect());
        p.setOpacity(1.0);
    }

    const std::vector<QPointF> P = currentPoints();
    const int m = static_cast<int>(P.size());
    const int n = m / 3;


    if (showPolygon_) {
        p.setPen(QPen(QColor(170, 170, 170), 1));
        for (int i = 0; i < n; ++i) {
            p.drawLine(P[3 * i], P[3 * i + 1]);
            p.drawLine(P[3 * i + 1], P[3 * i + 2]);
            p.drawLine(P[3 * i + 2], P[(3 * i + 3) % m]);
        }
    }


    p.setPen(QPen(Qt::black, 2));
    for (int i = 0; i < n; ++i) {
        QPolygonF poly;
        for (int k = 0; k <= 40; ++k)
            poly << bezier3(P[3 * i], P[3 * i + 1], P[3 * i + 2], P[(3 * i + 3) % m], k / 40.0);
        p.drawPolyline(poly);
    }


    if (showPolygon_) {
        p.setPen(Qt::NoPen);
        for (int i = 0; i < m; ++i) {
            const bool anchor = (i % 3 == 0);
            p.setBrush(anchor ? QColor(0, 0, 0) : QColor(40, 190, 40));
            p.drawEllipse(P[i], anchor ? 4.0 : 3.5, anchor ? 4.0 : 3.5);
        }
    }
}


int Canvas::hitTest(const QPointF& p) const {
    int best = -1;
    double bestD = 10.0;
    for (size_t i = 0; i < pts_.size(); ++i) {
        const QPointF d = pts_[i] - p;
        const double dist = std::hypot(d.x(), d.y());
        if (dist < bestD) { bestD = dist; best = static_cast<int>(i); }
    }
    return best;
}

void Canvas::mousePressEvent(QMouseEvent* e) {
    if (animating_ || !showPolygon_ || e->button() != Qt::LeftButton) return;
    drag_ = hitTest(e->position());
}

void Canvas::mouseMoveEvent(QMouseEvent* e) {
    if (drag_ < 0) return;
    const int m = static_cast<int>(pts_.size());
    const QPointF p = e->position();
    if (drag_ % 3 == 0) {

        const QPointF d = p - pts_[drag_];
        pts_[drag_] = p;
        pts_[(drag_ + 1) % m] += d;
        pts_[(drag_ + m - 1) % m] += d;
    } else {
        pts_[drag_] = p;
        if (smooth_) {  
            int anchor, opp;
            if (drag_ % 3 == 1) { anchor = drag_ - 1; opp = (drag_ - 2 + m) % m; }
            else                { anchor = (drag_ + 1) % m; opp = (drag_ + 2) % m; }
            pts_[opp] = 2 * pts_[anchor] - p;
        }
    }
    update();
}

void Canvas::mouseReleaseEvent(QMouseEvent*) { drag_ = -1; }
