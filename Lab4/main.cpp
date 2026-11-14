#include <QApplication>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QCheckBox>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include "Canvas.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("Лаб. 4 - Дельфін, гладкі контури з кривих Безьє (варіант 15)");
    auto* root = new QHBoxLayout(&window);

    auto* canvas = new Canvas;
    root->addWidget(canvas, 1);

    auto* panel = new QWidget;
    panel->setFixedWidth(230);
    auto* v = new QVBoxLayout(panel);
    root->addWidget(panel);

    auto* showPoly = new QCheckBox("Показати характеристичний\nбагатокутник");
    showPoly->setChecked(true);
    QObject::connect(showPoly, &QCheckBox::toggled, canvas, &Canvas::setShowPolygon);
    v->addWidget(showPoly);

    auto* smooth = new QCheckBox("Гладкість 1-го порядку\nу вузлових точках");
    smooth->setChecked(true);
    QObject::connect(smooth, &QCheckBox::toggled, canvas, &Canvas::setSmoothJoints);
    v->addWidget(smooth);

    v->addWidget(new QLabel("Натяг контрольних точок:"));
    auto* tension = new QSlider(Qt::Horizontal);
    tension->setRange(5, 250);
    tension->setValue(100);
    QObject::connect(tension, &QSlider::valueChanged, canvas, &Canvas::setTension);
    v->addWidget(tension);

    auto* anim = new QPushButton("Анімація (дельфін <-> трикутник)");
    QObject::connect(anim, &QPushButton::clicked, canvas, &Canvas::toggleAnimation);
    v->addWidget(anim);

    v->addWidget(new QLabel("Евклідові перетворення:"));
    auto* g = new QGridLayout;
    auto addBtn = [&](const QString& t, int r, int c, auto fn) {
        auto* b = new QPushButton(t);
        QObject::connect(b, &QPushButton::clicked, canvas, fn);
        g->addWidget(b, r, c);
    };
    addBtn("↑", 0, 1, [canvas] { canvas->move(0, -15); });
    addBtn("←", 1, 0, [canvas] { canvas->move(-15, 0); });
    addBtn("→", 1, 2, [canvas] { canvas->move(15, 0); });
    addBtn("↓", 2, 1, [canvas] { canvas->move(0, 15); });
    addBtn("⟲ 15°", 3, 0, [canvas] { canvas->rotate(-15); });
    addBtn("⟳ 15°", 3, 2, [canvas] { canvas->rotate(15); });
    addBtn("Відобр. ↔", 4, 0, [canvas] { canvas->flipH(); });
    addBtn("Відобр. ↕", 4, 2, [canvas] { canvas->flipV(); });
    v->addLayout(g);

    auto* bg = new QPushButton("Завантажити фон-картинку...");
    QObject::connect(bg, &QPushButton::clicked, canvas, &Canvas::loadBackground);
    v->addWidget(bg);

    auto* reset = new QPushButton("Скинути форму");
    QObject::connect(reset, &QPushButton::clicked, canvas, [canvas, tension] {
        canvas->resetShape();
        tension->setValue(100);
    });
    v->addWidget(reset);

    v->addStretch(1);
    v->addWidget(new QLabel("ЛКМ по точці - перетягування.\nЧорні - вузлові, зелені - реперні,\nчервоні - точки зламу.\nПодвійний клік по вузловій точці -\nвузлова <-> точка зламу."));

    window.resize(1100, 720);
    window.show();
    return app.exec();
}
