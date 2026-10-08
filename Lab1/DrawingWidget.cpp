#include "DrawingWidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <algorithm>
#include <cmath>

namespace {
constexpr double PI = 3.14159265358979323846;

// Положення осі отворів Ø10 (центр лівої дуги) на сцені, мм.
// Габарит деталі за X: від -35 до 95, тому центр габариту = 30; зсув -30 ставить
// центр деталі (перетин її осей симетрії) в початок координат.
constexpr double kModelX = -30.0;
constexpr double kModelY = 0.0;

// За зразком із ТЗ додатний кут обертання - за годинниковою стрілкою (на екрані).
constexpr bool kPositiveAngleIsClockwise = true;
}

DrawingWidget::DrawingWidget(QWidget* parent) : QWidget(parent) {
    setMinimumSize(820, 680);
    setMouseTracking(true);}

// перетворення
void DrawingWidget::resetTransform() {
    model_ = Matrix3::identity();
    coord_ = Matrix3::identity();
    update();
}

void DrawingWidget::translateModel(double dx, double dy) {
    model_ = Matrix3::translation(dx,dy) * model_;
    update();
}

void DrawingWidget::rotateModel(double px, double py, double angleDeg) {
    const double a = kPositiveAngleIsClockwise ? -angleDeg : angleDeg;
    model_ = Matrix3::aroundPoint(Matrix3::rotationDeg(a), px, py) * model_;
    pivot_ = QPointF(px,py);
    update();
}

void DrawingWidget::scaleModel(double px, double py, double sx, double sy) {
    model_ = Matrix3::aroundPoint(Matrix3::scale(sx,sy), px, py) * model_;
    pivot_ = QPointF(px,py);
    update();
}

void DrawingWidget::symmetryModel(double px, double py) {
    model_ = Matrix3::pointSymmetry(px,py) * model_;
    pivot_ = QPointF(px,py);
    update();
}

void DrawingWidget::setAffine(double Xx,double Xy,double Yx,double Yy,double Ox,double Oy) {
    coord_ = Matrix3::affine(Xx,Xy,Yx,Yy,Ox,Oy);
    update();
}

void DrawingWidget::setProjective(double Xx,double Xy,double wx,
                                  double Yx,double Yy,double wy,
                                  double Ox,double Oy,double w0) {
    coord_ = Matrix3::projective(Xx,Xy,wx,Yx,Yy,wy,Ox,Oy,w0);
    update();
}

void DrawingWidget::setPivot(double x, double y) {
    pivot_ = QPointF(x,y);
    update();
}

// допоміжні
QPointF DrawingWidget::worldToScreen(const QPointF& p) const {
    return QPointF(originPx_.x() + p.x()*pxPerMm_, originPx_.y() - p.y()*pxPerMm_);
}
QPointF DrawingWidget::screenToWorld(const QPointF& p) const {
    return QPointF((p.x()-originPx_.x())/pxPerMm_, (originPx_.y()-p.y())/pxPerMm_);
}

QVector<QPointF> DrawingWidget::line(const QPointF& a, const QPointF& b, int steps) {
    QVector<QPointF> v; v.reserve(steps+1);
    for(int i=0;i<=steps;++i){
        const double t=double(i)/double(steps);
        v.push_back(a + (b-a)*t);
    }
    return v;
}

QVector<QPointF> DrawingWidget::arc(double cx,double cy,double r,double a0,double a1,int steps) {
    QVector<QPointF> v; v.reserve(steps+1);
    for(int i=0;i<=steps;++i){
        const double t=a0+(a1-a0)*double(i)/double(steps);
        const double q=t*PI/180.0;
        v.push_back(QPointF(cx+r*std::cos(q), cy+r*std::sin(q)));
    }
    return v;
}

QVector<QPointF> DrawingWidget::circle(double cx,double cy,double r,int steps) {
    return arc(cx,cy,r,0.0,360.0,steps);
}

//  ламана: точки проходять (для деталі) model_ і далі coord_.
// Точки, що не мають образу (W менше або дорівнює 0), розривають лінію.
void DrawingWidget::strokePolyline(QPainter& p, const QVector<QPointF>& pts, bool applyModel, bool closed) const {
    if(pts.size()<2) return;
    const Matrix3 M = applyModel ? coord_*model_ : coord_;
    const double rs = refSign();
    QPainterPath path;
    bool penDown=false, allOk=true;
    for(const QPointF& pt : pts){
        QPointF w;
        if(!M.mapChecked(pt,w,rs)) { penDown=false; allOk=false; continue; }
        const QPointF s = worldToScreen(w);
        if(std::abs(s.x())>1e5 || std::abs(s.y())>1e5) { penDown=false; allOk=false; continue; }
        if(!penDown){ path.moveTo(s); penDown=true; } else path.lineTo(s);
    }
    if(closed && allOk) path.closeSubpath();
    p.drawPath(path);
}


// Сітка та осі - в системі координат coord_ (для афінних/проективних перетворень
// змінюється система координат, а отже і сітка).
void DrawingWidget::drawGrid(QPainter& p) const {
    QPen grid(QColor(215,215,215)); grid.setWidthF(1.0);
    p.setPen(grid);
    for(double x=-gridMaxX_; x<=gridMaxX_+1e-9; x+=10.0)
        strokePolyline(p, line(QPointF(x,-gridMaxY_), QPointF(x,gridMaxY_), 80), false);
    for(double y=-gridMaxY_; y<=gridMaxY_+1e-9; y+=10.0)
        strokePolyline(p, line(QPointF(-gridMaxX_,y), QPointF(gridMaxX_,y), 80), false);
}

void DrawingWidget::drawAxes(QPainter& p) const {
    QPen axis(QColor(60,60,60)); axis.setWidthF(1.6);
    p.setPen(axis);
    const QPointF ex(gridMaxX_,0), ey(0,gridMaxY_);
    strokePolyline(p, line(QPointF(-gridMaxX_,0), ex, 80), false);
    strokePolyline(p, line(QPointF(0,-gridMaxY_), ey, 80), false);

    // стрілки на кінцях осей (напрямок береться з екранного образу осі)
    const double rs = refSign();
    auto arrow = [&](const QPointF& tip, const QPointF& back, const QString& label){
        QPointF a,b;
        if(!coord_.mapChecked(tip,a,rs) || !coord_.mapChecked(back,b,rs)) return;
        const QPointF st=worldToScreen(a), sb=worldToScreen(b);
        QPointF d=st-sb; const double len=std::hypot(d.x(),d.y());
        if(len<1e-6) return;
        d/=len; const QPointF n(-d.y(), d.x());
        p.drawLine(st, st - d*11.0 + n*5.0);
        p.drawLine(st, st - d*11.0 - n*5.0);
        p.drawText(st + QPointF(6,-6), label);
    };
    arrow(ex, QPointF(gridMaxX_-3,0), "X");
    arrow(ey, QPointF(0,gridMaxY_-3), "Y");

    // підписи "0" та "10"
    QPointF o,tx,ty;
    p.setPen(QColor(60,60,60));
    if(coord_.mapChecked(QPointF(0,0),o,rs)) p.drawText(worldToScreen(o)+QPointF(-12,14), "0");
    if(coord_.mapChecked(QPointF(10,0),tx,rs)) p.drawText(worldToScreen(tx)+QPointF(-6,14), "10");
    if(coord_.mapChecked(QPointF(0,10),ty,rs)) p.drawText(worldToScreen(ty)+QPointF(-20,4), "10");
}

// Побудова за кресленням варіанта 15 (мм). Початок координат моделі - центр лівої
// дуги R35 = вісь двох отворів Ø10.
//   - ліва дуга R (центр (0,0)), права дуга R (центр (60,0));
//   - верхній/нижній відрізки - зовнішні дотичні до обох дуг (при рівних R - горизонтальні),
//     тому при зміні довжини чи радіусів вони автоматично з'єднуються з новими кінцями дуг;
//   - лівий виступ висотою 20, права стінка на відстані 10 від осі Ø10;
//   - два отвори Ø10 на осі x=0, центри на відстані 50;
//   - прямокутник 50x40 з центром у центрі Ø20 і дві "вилки" 16x5;
//   - отвір Ø20 у центрі правої дуги.
void DrawingWidget::drawModel(QPainter& p) const {
    QPen pen(Qt::black); pen.setWidthF(2.0);
    p.setPen(pen); p.setBrush(Qt::NoBrush);

    auto P = [](double x,double y){ return QPointF(kModelX + x, kModelY + y); };

    const double RL = std::max(1.0, outerRLeft);
    const double RR = std::max(1.0, outerRRight);
    const double D  = std::max(1.0, straightLen);
    const double cL = 0.0, cR = D;

    // Зовнішня дотична: n=(cos t, sin t), (RL-RR) = D*cos t
    const double ct = std::clamp((RL-RR)/D, -1.0, 1.0);
    const double th = std::acos(ct);                 // рад
    const double thDeg = th*180.0/PI;

    // Зовнішній контур
    strokePolyline(p, arc(kModelX+cL, kModelY, RL, thDeg, 360.0-thDeg), true);
    strokePolyline(p, arc(kModelX+cR, kModelY, RR, -thDeg, thDeg), true);
    strokePolyline(p, line(P(cL+RL*std::cos(th),  RL*std::sin(th)),
                           P(cR+RR*std::cos(th),  RR*std::sin(th))), true);
    strokePolyline(p, line(P(cL+RL*std::cos(th), -RL*std::sin(th)),
                           P(cR+RR*std::cos(th), -RR*std::sin(th))), true);

    // Лівий виступ висотою 20 (права стінка на відстані topOffset від осі Ø10)
    const double nh = std::min(leftNotchHeight/2.0, RL*0.999);
    const double notchLeft  = cL - std::sqrt(std::max(0.0, RL*RL - nh*nh));
    const double notchRight = std::max(notchLeft, cL - topOffset);
    strokePolyline(p, QVector<QPointF>{P(notchLeft, nh), P(notchRight, nh),
                                       P(notchRight,-nh), P(notchLeft,-nh)}, true);

    // Два отвори Ø10, відстань між центрами 50
    const double hy = smallHoleSpacing/2.0;
    strokePolyline(p, circle(kModelX+cL, kModelY+hy, smallHoleD/2.0), true, true);
    strokePolyline(p, circle(kModelX+cL, kModelY-hy, smallHoleD/2.0), true, true);

    // Прямокутник 50x40 та дві "вилки" 16x5
    const double hw = innerWidth/2.0, hh = innerHeight/2.0;
    const double leftEdge = cR - hw, rightEdge = cR + hw;
    const double t = std::min(forkThickness, hh);
    const double forkX = leftEdge - forkLen;
    const double gapY = hh - t;
    strokePolyline(p, QVector<QPointF>{
        P(forkX, hh), P(rightEdge, hh), P(rightEdge,-hh), P(forkX,-hh),
        P(forkX,-gapY), P(leftEdge,-gapY), P(leftEdge, gapY), P(forkX, gapY), P(forkX, hh)}, true);
    strokePolyline(p, QVector<QPointF>{P(leftEdge, hh), P(leftEdge, gapY)}, true);
    strokePolyline(p, QVector<QPointF>{P(leftEdge,-gapY), P(leftEdge,-hh)}, true);

    // Отвір Ø20
    strokePolyline(p, circle(kModelX+cR, kModelY, bigHoleD/2.0), true, true);

    // Осьові штрих-пунктирні лінії
    QPen cp(QColor(120,120,120)); cp.setStyle(Qt::DashDotLine); cp.setWidthF(1.0);
    p.setPen(cp);
    const double Rm = std::max(RL,RR);
    strokePolyline(p, line(P(cL-RL-5,0), P(cR+RR+5,0), 60), true);
    strokePolyline(p, line(P(cL,-Rm-4), P(cL,Rm+4), 40), true);
    strokePolyline(p, line(P(cR,-hh-8), P(cR,hh+8), 40), true);
}

// Точка-центр обертання/масштабування/симетрії (задається користувачем).
// Вона задана в координатах системи coord_ (тому відображається через coord_).
void DrawingWidget::drawPivot(QPainter& p) const {
    QPointF w;
    if(!coord_.mapChecked(pivot_, w, refSign())) return;
    const QPointF c = worldToScreen(w);
    p.setPen(QPen(QColor(200,30,30), 1.5));
    p.setBrush(QColor(200,30,30));
    p.drawEllipse(c, 3.0, 3.0);
    p.setBrush(Qt::NoBrush);
}

void DrawingWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing,true);
    p.fillRect(rect(),Qt::white);

    // Початок координат - у центрі вікна; сітка й осі на все вікно (усі чотири квадранти)
    originPx_ = QPointF(width()/2.0, height()/2.0);
    gridMaxX_ = std::max(50.0, std::floor((width()/2.0  - 12.0)/pxPerMm_/10.0)*10.0);
    gridMaxY_ = std::max(50.0, std::floor((height()/2.0 - 12.0)/pxPerMm_/10.0)*10.0);

    drawGrid(p);
    drawAxes(p);
    drawModel(p);
    drawPivot(p);
}

void DrawingWidget::mouseMoveEvent(QMouseEvent* e) {
    // піксель -> координати у вибраній системі (через обернену матрицю coord_)
    QPointF w = screenToWorld(e->position());
    bool ok=false;
    const Matrix3 inv = coord_.inverse(&ok);
    if(ok){
        const QPointF q = inv.map(w);
        if(std::isfinite(q.x()) && std::isfinite(q.y())) w = q;
    }
    emit mouseWorldPosition(w.x(),w.y());
}
