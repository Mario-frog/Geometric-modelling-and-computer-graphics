#include <QApplication>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <array>
#include "Canvas.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("Лаб. 5 - Грановане тіло, ортографічне проекціювання (варіант 15, метод 1)");
    auto* root = new QHBoxLayout(&window);

    auto* canvas = new Canvas;
    root->addWidget(canvas, 1);

    auto* panel = new QWidget;
    panel->setFixedWidth(310);
    auto* col = new QVBoxLayout(panel);
    root->addWidget(panel);

    auto makeSpin = [](int lo, int hi, int val) {
        auto* s = new QSpinBox;
        s->setRange(lo, hi);
        s->setValue(val);
        return s;
    };

    // Проекціювання
    auto* gProj = new QGroupBox("Проекціювання");
    auto* lp = new QVBoxLayout(gProj);
    auto* view = new QComboBox;
    view->addItems({"Площина x = 0 (збоку)", "Площина y = 0 (зверху)",
                    "Площина z = 0 (спереду)", "Усі три площини"});
    view->setCurrentIndex(3);
    lp->addWidget(view);
    QObject::connect(view, &QComboBox::currentIndexChanged, canvas, &Canvas::setView);
    col->addWidget(gProj);

    // Параметри фігури
    auto* gFig = new QGroupBox("Параметри фігури");
    auto* gf = new QGridLayout(gFig);
    auto* sR = makeSpin(10, 150, 60);
    auto* sH = makeSpin(10, 250, 120);
    auto* sK = makeSpin(10, 90, 40);
    auto* sS = makeSpin(20, 300, 100);
    gf->addWidget(new QLabel("Радіус"), 0, 0);            gf->addWidget(sR, 0, 1);
    gf->addWidget(new QLabel("Висота"), 1, 0);            gf->addWidget(sH, 1, 1);
    gf->addWidget(new QLabel("Глибина зірки, %"), 2, 0);  gf->addWidget(sK, 2, 1);
    gf->addWidget(new QLabel("Масштаб, %"), 3, 0);        gf->addWidget(sS, 3, 1);
    QObject::connect(sR, &QSpinBox::valueChanged, canvas, [canvas](int v) { canvas->setRadius(v); });
    QObject::connect(sH, &QSpinBox::valueChanged, canvas, [canvas](int v) { canvas->setHeight(v); });
    QObject::connect(sK, &QSpinBox::valueChanged, canvas, [canvas](int v) { canvas->setInner(v); });
    QObject::connect(sS, &QSpinBox::valueChanged, canvas, [canvas](int v) { canvas->setScale(v); });
    col->addWidget(gFig);

    // Евклідові перетворення
    auto* gTr = new QGroupBox("Евклідові перетворення");
    auto* gt = new QGridLayout(gTr);
    const char* axisNames[3] = {"X", "Y", "Z"};
    for (int i = 0; i < 3; ++i) gt->addWidget(new QLabel(axisNames[i]), 0, i + 1, Qt::AlignCenter);
    std::array<QSpinBox*, 3> tPos, tRot;
    gt->addWidget(new QLabel("Зсув"), 1, 0);
    gt->addWidget(new QLabel("Оберт.,°"), 2, 0);
    for (int i = 0; i < 3; ++i) {
        tPos[i] = makeSpin(-250, 250, 0);
        tRot[i] = makeSpin(-360, 360, 0);
        gt->addWidget(tPos[i], 1, i + 1);
        gt->addWidget(tRot[i], 2, i + 1);
    }
    auto* bApply = new QPushButton("Застосувати");
    auto* bDefault = new QPushButton("За замовчуванням");
    gt->addWidget(bApply, 3, 0, 1, 2);
    gt->addWidget(bDefault, 3, 2, 1, 2);
    gt->addWidget(new QLabel("Обертання - навколо власного центру фігури"), 4, 0, 1, 4);
    col->addWidget(gTr);

    QObject::connect(bApply, &QPushButton::clicked, canvas, [=] {
        canvas->applyTransform(
            {double(tPos[0]->value()), double(tPos[1]->value()), double(tPos[2]->value())},
            {double(tRot[0]->value()), double(tRot[1]->value()), double(tRot[2]->value())});
    });
    QObject::connect(bDefault, &QPushButton::clicked, canvas, [=] {
        for (int i = 0; i < 3; ++i) { tPos[i]->setValue(0); tRot[i]->setValue(0); }
        sR->setValue(60); sH->setValue(120); sK->setValue(40); sS->setValue(100);
        canvas->resetAll();
    });

    //Анімація
    auto* gAn = new QGroupBox("Анімація");
    auto* ga = new QGridLayout(gAn);
    for (int i = 0; i < 3; ++i) ga->addWidget(new QLabel(axisNames[i]), 0, i + 1, Qt::AlignCenter);
    std::array<QSpinBox*, 3> aPos, aRot;
    ga->addWidget(new QLabel("Крок зсуву"), 1, 0);
    ga->addWidget(new QLabel("Крок оберт.,°"), 2, 0);
    const int defPos[3] = {1, 0, 0}, defRot[3] = {0, 3, 0};
    for (int i = 0; i < 3; ++i) {
        aPos[i] = makeSpin(-20, 20, defPos[i]);
        aRot[i] = makeSpin(-30, 30, defRot[i]);
        ga->addWidget(aPos[i], 1, i + 1);
        ga->addWidget(aRot[i], 2, i + 1);
    }
    auto* hMin = makeSpin(10, 250, 60);
    auto* hMax = makeSpin(10, 250, 200);
    ga->addWidget(new QLabel("Висота мін / макс"), 3, 0);
    ga->addWidget(hMin, 3, 1);
    ga->addWidget(hMax, 3, 2);
    auto* bAnim = new QPushButton("Старт анімації");
    ga->addWidget(bAnim, 4, 0, 1, 4);
    col->addWidget(gAn);

    auto pushAnim = [=] {
        canvas->setAnimation(
            {double(aPos[0]->value()), double(aPos[1]->value()), double(aPos[2]->value())},
            {double(aRot[0]->value()), double(aRot[1]->value()), double(aRot[2]->value())},
            hMin->value(), hMax->value());
    };
    for (int i = 0; i < 3; ++i) {
        QObject::connect(aPos[i], &QSpinBox::valueChanged, canvas, [=](int) { pushAnim(); });
        QObject::connect(aRot[i], &QSpinBox::valueChanged, canvas, [=](int) { pushAnim(); });
    }
    QObject::connect(hMin, &QSpinBox::valueChanged, canvas, [=](int) { pushAnim(); });
    QObject::connect(hMax, &QSpinBox::valueChanged, canvas, [=](int) { pushAnim(); });
    pushAnim();

    QObject::connect(bAnim, &QPushButton::clicked, canvas, [=] {
        pushAnim();
        const bool running = canvas->toggleAnimation();
        bAnim->setText(running ? "Стоп анімації" : "Старт анімації");
        bApply->setEnabled(!running);
        bDefault->setEnabled(!running);
    });

    col->addStretch(1);
    window.resize(1200, 760);
    window.show();
    return app.exec();
}
