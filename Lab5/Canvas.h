#pragma once
#include <QWidget>
#include <QTimer>
#include <QString>
#include <array>
#include <vector>
#include <utility>
#include "Projection.h"

class Canvas : public QWidget {
    Q_OBJECT
public:
    explicit Canvas(QWidget* parent = nullptr);

    // 0: x=0 (збоку), 1: y=0 (зверху), 2: z=0 (спереду), 3: усі три площини
    void setView(int index);

    // параметри гранованого тіла (зірка-призма)
    void setRadius(double r);
    void setHeight(double h);
    void setInner(double pct);     // глибина зірки: внутрішній радіус у % від зовнішнього
    void setScale(double pct);     // масштаб у %, застосовується матрицею масштабування

    // евклідові перетворення: абсолютний зсув та кути обертання навколо власного центру
    void applyTransform(const std::array<double, 3>& pos, const std::array<double, 3>& rotDeg);
    void resetAll();

    // анімація: крок зсуву/обертання за кадр + діапазон зміни висоти
    void setAnimation(const std::array<double, 3>& dPos, const std::array<double, 3>& dRotDeg,
                      double hMin, double hMax);
    bool toggleAnimation();        // повертає true, якщо анімація запущена

protected:
    void paintEvent(QPaintEvent*) override;

private:
    void tick();
    double currentHeight() const { return useAnimH_ ? animH_ : height_; }
    std::vector<Vec4> bodyVertices(double H) const;
    Mat4 modelMatrix() const;
    void drawView(QPainter& p, const QRectF& vp, int plane, const QString& title) const;

    // параметри фігури
    double radius_ = 60, height_ = 120, inner_ = 40, scale_ = 100;
    // положення та орієнтація
    std::array<double, 3> pos_{0, 0, 0}, rot_{0, 0, 0};
    // анімація
    QTimer timer_;
    bool animating_ = false, useAnimH_ = false;
    std::array<double, 3> dPos_{1, 0, 0}, dRot_{0, 3, 0};
    std::array<double, 3> savedPos_{}, savedRot_{}, sign_{1, 1, 1};
    double hMin_ = 60, hMax_ = 200, animH_ = 120;
    int hDir_ = 1;

    int view_ = 3;
};
