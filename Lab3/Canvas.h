#pragma once
#include <QWidget>
#include <QPointF>
#include <QPixmap>
#include <QTimer>
#include <QTransform>
#include <vector>

// Замкнутый контур из N кубических кривых Безье.
// pts_[3i]   - опорная точка A_i
// pts_[3i+1] - первая контрольная точка сегмента i
// pts_[3i+2] - вторая контрольная точка сегмента i
// конец сегмента i = pts_[(3i+3) % (3N)] (следующая опорная точка)
class Canvas : public QWidget {
    Q_OBJECT
public:
    explicit Canvas(QWidget* parent = nullptr);

public slots:
    void setShowPolygon(bool on);
    void setSmoothJoints(bool on);
    void setTension(int percent);   // 100 = 1.0
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

private:
    std::vector<QPointF> currentPoints() const;
    std::vector<QPointF> buildTriangleTarget() const;
    QPointF centroid() const;
    int hitTest(const QPointF& p) const;

    std::vector<QPointF> pts_;
    std::vector<QPointF> target_;
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
