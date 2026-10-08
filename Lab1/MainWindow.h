#pragma once
#include <QMainWindow>
class QDoubleSpinBox;
class QLabel;
class DrawingWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow();
private:
    DrawingWidget* view_{};
    QLabel* status_{};
    static QDoubleSpinBox* spin(double v,double min=-1000,double max=1000,double step=1,int decimals=3);
};
