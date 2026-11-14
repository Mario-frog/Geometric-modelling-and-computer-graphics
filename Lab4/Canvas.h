#pragma once
#include <QWidget>
#include <QPointF>
#include <QPixmap>
#include <QTimer>
#include <QTransform>
#include <vector>


class Canvas : public QWidget {
    Q_OBJECT
public:
    explicit Canvas(QWidget* parent = nullptr);

public slots:
    void setShowPolygon(bool on);
    void setSmoothJoints(bool on);
    void setTension(int percent);  
    void toggleAnimation();
    void loadBackground();
    void rotate(double deg);
    void flipH();
    void flipV();
    void move(double dx, double dy);
    void resetShape();

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void mouseDoubleClickEvent(QMouseEvent*) override;

private:
    std::vector<QPointF> currentPoints() const;
    std::vector<QPointF> buildTriangleTarget() const;
    QPointF centroid() const;
    void applyTransform(const QTransform& t);
    int hitTest(const QPointF& p) const;
    void enforceSmooth(int joint, bool keepLeft);

    std::vector<QPointF> pts_;
    std::vector<QPointF> target_;
    std::vector<bool> corner_; 
    QPixmap bg_;
    QTimer timer_;
    bool showPolygon_ = true;
    bool smooth_ = true;
    bool animating_ = false;
    double tension_ = 1.0;
    double phase_ = 0.0;
    int dir_ = 1;
    double s_ = 0.0;
    int drag_ = -1;
};
