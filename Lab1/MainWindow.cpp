#include "MainWindow.h"
#include "DrawingWidget.h"
#include <QtWidgets>

QDoubleSpinBox* MainWindow::spin(double v,double min,double max,double step,int decimals){
    auto* s=new QDoubleSpinBox;
    s->setRange(min,max); s->setDecimals(decimals); s->setSingleStep(step); s->setValue(v);
    s->setMinimumWidth(78);
    return s;
}

MainWindow::MainWindow(){
    setWindowTitle("ЛР №1 Лінійні перетворення Варіант 15");
    resize(1380,820);

    auto* central=new QWidget; setCentralWidget(central);
    auto* root=new QHBoxLayout(central);
    view_=new DrawingWidget; root->addWidget(view_,1);

    auto* scroll=new QScrollArea; scroll->setWidgetResizable(true); scroll->setFixedWidth(430);
    auto* panel=new QWidget; auto* lay=new QVBoxLayout(panel); scroll->setWidget(panel); root->addWidget(scroll);

    auto form=[&](const QString& t){ auto* g=new QGroupBox(t); auto* f=new QFormLayout(g); lay->addWidget(g); return f; };
    auto grid=[&](const QString& t){ auto* g=new QGroupBox(t); auto* f=new QGridLayout(g); lay->addWidget(g); return f; };

    //  1-2. Параметри деталі
    // Зміна будь-якого розміру одразу перебудовує деталь: сусідні елементи
    // з'єднуються з новими кінцями дуги/відрізка.
    auto* gd=form("Параметри деталі (варіант 15), мм");
    auto* dimRL=spin(35,1,200,1,2);   auto* dimRR=spin(35,1,200,1,2);
    auto* dimL =spin(60,1,300,1,2);   auto* dimOff=spin(10,0,100,1,2);
    auto* dimD20=spin(20,1,100,1,2);  auto* dimD10=spin(10,1,100,1,2);
    auto* dimHole=spin(50,0,200,1,2); auto* dimH40=spin(40,1,200,1,2);
    auto* dimW50=spin(50,1,250,1,2);  auto* dimNotch=spin(20,1,150,1,2);
    auto* dimFork=spin(16,0,100,1,2); auto* dimGap=spin(5,0,100,1,2);
    gd->addRow("R лівої дуги (35)",dimRL);
    gd->addRow("R правої дуги (35)",dimRR);
    gd->addRow("Вісь Ø10 → центр Ø20 (60)",dimL);
    gd->addRow("Вісь Ø10 → виступ (10)",dimOff);
    gd->addRow("Ø20",dimD20);
    gd->addRow("Ø10 (2 отв.)",dimD10);
    gd->addRow("Між центрами Ø10 (50)",dimHole);
    gd->addRow("Висота прямокутника (40)",dimH40);
    gd->addRow("Ширина прямокутника (50)",dimW50);
    gd->addRow("Висота лівого виступу (20)",dimNotch);
    gd->addRow("Довжина вилки (16)",dimFork);
    gd->addRow("Товщина вилки (5)",dimGap);
    auto applyDims=[=]{
        view_->outerRLeft=dimRL->value();  view_->outerRRight=dimRR->value();
        view_->straightLen=dimL->value();  view_->topOffset=dimOff->value();
        view_->bigHoleD=dimD20->value();   view_->smallHoleD=dimD10->value();
        view_->smallHoleSpacing=dimHole->value();
        view_->innerHeight=dimH40->value(); view_->innerWidth=dimW50->value();
        view_->leftNotchHeight=dimNotch->value();
        view_->forkLen=dimFork->value();   view_->forkThickness=dimGap->value();
        view_->update();
    };
    for(auto* s : {dimRL,dimRR,dimL,dimOff,dimD20,dimD10,dimHole,dimH40,dimW50,dimNotch,dimFork,dimGap})
        connect(s,&QDoubleSpinBox::valueChanged,this,applyDims);

    //  3.1 Евклідові: зсув
    auto* gt=grid("Translation (зсув)");
    auto* dx=spin(10,-1000,1000,5), *dy=spin(10,-1000,1000,5);
    gt->addWidget(new QLabel("dx"),0,0); gt->addWidget(dx,0,1);
    gt->addWidget(new QLabel("dy"),0,2); gt->addWidget(dy,0,3);
    auto* bt=new QPushButton("Apply"); gt->addWidget(bt,1,0,1,4);
    connect(bt,&QPushButton::clicked,this,[=]{view_->translateModel(dx->value(),dy->value());});

    //  3.1 Евклідові: обертання
    auto* gr=grid("Rotation (обертання навколо точки)");
    auto* rx=spin(0,-1000,1000,5), *ry=spin(0,-1000,1000,5), *ang=spin(45,-360,360,5);
    gr->addWidget(new QLabel("x"),0,0);     gr->addWidget(rx,0,1);
    gr->addWidget(new QLabel("y"),0,2);     gr->addWidget(ry,0,3);
    gr->addWidget(new QLabel("angle"),0,4); gr->addWidget(ang,0,5);
    auto* br=new QPushButton("Apply"); gr->addWidget(br,1,0,1,6);
    connect(br,&QPushButton::clicked,this,[=]{view_->rotateModel(rx->value(),ry->value(),ang->value());});
    auto showRot=[=]{ view_->setPivot(rx->value(),ry->value()); };
    connect(rx,&QDoubleSpinBox::valueChanged,this,showRot);
    connect(ry,&QDoubleSpinBox::valueChanged,this,showRot);

    //  3.2 Масштаб та симетрія
    auto* gs=grid("Scaling / Symmetry (масштаб, симетрія відносно точки)");
    auto* sxp=spin(0,-1000,1000,5), *syp=spin(0,-1000,1000,5);
    auto* sx=spin(1.5,-100,100,0.1), *sy=spin(1.5,-100,100,0.1);
    gs->addWidget(new QLabel("x"),0,0);  gs->addWidget(sxp,0,1);
    gs->addWidget(new QLabel("y"),0,2);  gs->addWidget(syp,0,3);
    gs->addWidget(new QLabel("Sx"),1,0); gs->addWidget(sx,1,1);
    gs->addWidget(new QLabel("Sy"),1,2); gs->addWidget(sy,1,3);
    auto* bsc=new QPushButton("Scale"); gs->addWidget(bsc,2,0,1,2);
    auto* bsm=new QPushButton("Symmetry"); gs->addWidget(bsm,2,2,1,2);
    connect(bsc,&QPushButton::clicked,this,[=]{view_->scaleModel(sxp->value(),syp->value(),sx->value(),sy->value());});
    connect(bsm,&QPushButton::clicked,this,[=]{view_->symmetryModel(sxp->value(),syp->value());});
    auto showSc=[=]{ view_->setPivot(sxp->value(),syp->value()); };
    connect(sxp,&QDoubleSpinBox::valueChanged,this,showSc);
    connect(syp,&QDoubleSpinBox::valueChanged,this,showSc);

    //  3.3 Афінні (нова система координат)
    // Рядки матриці: X-вектор, Y-вектор, початок O.
    auto* ga=grid("Affine (нова система координат)");
    auto* axx=spin(1,-10000,10000,0.1), *axy=spin(0,-10000,10000,0.1);
    auto* ayx=spin(0,-10000,10000,0.1), *ayy=spin(1,-10000,10000,0.1);
    auto* aox=spin(0,-10000,10000,1),   *aoy=spin(0,-10000,10000,1);
    ga->addWidget(new QLabel("Xx"),0,0); ga->addWidget(axx,0,1); ga->addWidget(new QLabel("Xy"),0,2); ga->addWidget(axy,0,3);
    ga->addWidget(new QLabel("Yx"),1,0); ga->addWidget(ayx,1,1); ga->addWidget(new QLabel("Yy"),1,2); ga->addWidget(ayy,1,3);
    ga->addWidget(new QLabel("0x"),2,0); ga->addWidget(aox,2,1); ga->addWidget(new QLabel("0y"),2,2); ga->addWidget(aoy,2,3);
    auto* ba=new QPushButton("Apply"); ga->addWidget(ba,3,0,1,4);
    connect(ba,&QPushButton::clicked,this,[=]{
        view_->setAffine(axx->value(),axy->value(),ayx->value(),ayy->value(),aox->value(),aoy->value());});

    //  3.4 Проективні (нова система координат)
    auto* gp=grid("Projective (нова система координат)");
    auto* pxx=spin(1000,-10000,10000,10), *pxy=spin(0,-10000,10000,0.1), *pwx=spin(0.001,-10000,10000,0.001);
    auto* pyx=spin(0,-10000,10000,0.1), *pyy=spin(1000,-10000,10000,10), *pwy=spin(0.001,-10000,10000,0.001);
    auto* pox=spin(0,-10000,10000,1),   *poy=spin(0,-10000,10000,1),   *pw0=spin(1,-10000,10000,0.1);
    // Ваги wX,wY множать вектори осей (x'=Xx*wX*x/(wX*x+wY*y+w0)), тому wX=wY=0 вироджує матрицю.
    // Типові значення: Xx=Yy=1000, wX=wY=0.001, w0=1 ≈ тотожне перетворення з легкою перспективою.
    gp->addWidget(new QLabel("Xx"),0,0); gp->addWidget(pxx,0,1); gp->addWidget(new QLabel("Xy"),0,2); gp->addWidget(pxy,0,3); gp->addWidget(new QLabel("wX"),0,4); gp->addWidget(pwx,0,5);
    gp->addWidget(new QLabel("Yx"),1,0); gp->addWidget(pyx,1,1); gp->addWidget(new QLabel("Yy"),1,2); gp->addWidget(pyy,1,3); gp->addWidget(new QLabel("wY"),1,4); gp->addWidget(pwy,1,5);
    gp->addWidget(new QLabel("0x"),2,0); gp->addWidget(pox,2,1); gp->addWidget(new QLabel("0y"),2,2); gp->addWidget(poy,2,3); gp->addWidget(new QLabel("w0"),2,4); gp->addWidget(pw0,2,5);
    auto* bp=new QPushButton("Apply"); gp->addWidget(bp,3,0,1,6);
    connect(bp,&QPushButton::clicked,this,[=]{
        view_->setProjective(pxx->value(),pxy->value(),pwx->value(),
                             pyx->value(),pyy->value(),pwy->value(),
                             pox->value(),poy->value(),pw0->value());});

    auto* reset=new QPushButton("СКИНУТИ ВСІ ПЕРЕТВОРЕННЯ"); lay->addWidget(reset);
    connect(reset,&QPushButton::clicked,view_,&DrawingWidget::resetTransform);
    status_=new QLabel("Курсор: x=0 мм, y=0 мм"); lay->addWidget(status_); lay->addStretch();
    connect(view_,&DrawingWidget::mouseWorldPosition,this,[=](double x,double y){
        status_->setText(QString("Курсор: x=%1 мм, y=%2 мм").arg(x,0,'f',1).arg(y,0,'f',1));});
}
