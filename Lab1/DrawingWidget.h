#pragma once
#include <QWidget>
#include <QVector>
#include <QPointF>
#include "Matrix3.h"

class DrawingWidget : public QWidget {
    Q_OBJECT
public:
    explicit DrawingWidget(QWidget* parent=nullptr);

    //  Параметри деталі, варіант 15 (усі розміри в мм)
    double outerRLeft  = 35.0;         // R35 - ліва дуга, центр на осі отворів Ø10
    double outerRRight = 35.0;         // R35 - права дуга, центр у центрі Ø20
    double straightLen = 60.0;         // 60: вісь Ø10 -> центр Ø20
    double topOffset = 10.0;           // 10: вісь Ø10 -> права стінка лівого виступу
    double bigHoleD = 20.0;            // Ø20
    double smallHoleD = 10.0;          // Ø10 (2 отвори)
    double smallHoleSpacing = 50.0;    // 50 між центрами Ø10
    double innerHeight = 40.0;         // 40
    double innerWidth = 50.0;          // 50
    double leftNotchHeight = 20.0;     // 20
    double forkLen = 16.0;             // 16
    double forkThickness = 5.0;        // 5

    // Евклідові перетворення та окремі випадки афінних (діють на деталь)
    void resetTransform();
    void translateModel(double dx, double dy);
    void rotateModel(double px, double py, double angleDeg);
    void scaleModel(double px, double py, double sx, double sy);
    void symmetryModel(double px, double py);

    // Системи координат (діють і на сітку з осями, і на деталь)
    void setAffine(double Xx, double Xy, double Yx, double Yy, double Ox, double Oy);
    void setProjective(double Xx, double Xy, double wx,
                       double Yx, double Yy, double wy,
                       double Ox, double Oy, double w0);

    // Точка, що відображається на сцені (центр обертання/масштабування/симетрії)
    void setPivot(double x, double y);

signals:
    void mouseWorldPosition(double x, double y);

protected:
    void paintEvent(QPaintEvent*) override;
    void mouseMoveEvent(QMouseEvent* e) override;

private:
    Matrix3 model_ = Matrix3::identity();   // перетворення тільки деталі
    Matrix3 coord_ = Matrix3::identity();   // поточна система координат (сітка + деталь)
    QPointF pivot_{0.0, 0.0};

    double pxPerMm_ = 3.5;
    QPointF originPx_;
    double gridMaxX_ = 200.0, gridMaxY_ = 160.0;

    QPointF worldToScreen(const QPointF& p) const;
    QPointF screenToWorld(const QPointF& p) const;
    double  refSign() const { return coord_.m[2][2] < 0.0 ? -1.0 : 1.0; }

    void strokePolyline(QPainter& p, const QVector<QPointF>& pts, bool applyModel, bool closed=false) const;
    void drawGrid(QPainter& p) const;
    void drawAxes(QPainter& p) const;
    void drawModel(QPainter& p) const;
    void drawPivot(QPainter& p) const;

    static QVector<QPointF> line(const QPointF& a, const QPointF& b, int steps=32);
    static QVector<QPointF> arc(double cx, double cy, double r, double a0, double a1, int steps=90);
    static QVector<QPointF> circle(double cx, double cy, double r, int steps=120);
};
