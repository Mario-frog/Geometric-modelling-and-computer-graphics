#pragma once
#include <QPointF>
#include <cmath>

//  реалізація матриці 3x3 для однорідних координат.
// Конвенція: точка - стовпчик (x, y, 1)^T, образ p' = M * p.
// Композиція A*B означає "спочатку B, потім A".
struct Matrix3 {
    double m[3][3]{};

    static Matrix3 identity() {
        Matrix3 r{};
        r.m[0][0]=r.m[1][1]=r.m[2][2]=1.0;
        return r;
    }

    static Matrix3 translation(double dx,double dy) {
        Matrix3 r=identity();
        r.m[0][2]=dx; r.m[1][2]=dy;
        return r;
    }

    static Matrix3 scale(double sx,double sy) {
        Matrix3 r=identity();
        r.m[0][0]=sx; r.m[1][1]=sy;
        return r;
    }

    // Стандартне математичне обертання (проти годинникової стрілки при осі Y вгору).
    static Matrix3 rotationDeg(double angleDeg) {
        constexpr double PI=3.14159265358979323846;
        const double a=angleDeg*PI/180.0;
        Matrix3 r=identity();
        r.m[0][0]= std::cos(a); r.m[0][1]=-std::sin(a);
        r.m[1][0]= std::sin(a); r.m[1][1]= std::cos(a);
        return r;
    }

    // Перетворення base відносно точки (px,py): T(p) * base * T(-p)
    static Matrix3 aroundPoint(const Matrix3& base,double px,double py) {
        return translation(px,py)*base*translation(-px,-py);
    }

    // Симетрія відносно точки = масштабування (-1,-1) відносно цієї точки
    static Matrix3 pointSymmetry(double px,double py) {
        return aroundPoint(scale(-1.0,-1.0),px,py);
    }

    // Афінна система координат: вектори осей X=(Xx,Xy), Y=(Yx,Yy) і початок O=(Ox,Oy).
    //   x' = Xx*x + Yx*y + Ox
    //   y' = Xy*x + Yy*y + Oy
    static Matrix3 affine(double Xx,double Xy,double Yx,double Yy,double Ox,double Oy) {
        Matrix3 r=identity();
        r.m[0][0]=Xx; r.m[0][1]=Yx; r.m[0][2]=Ox;
        r.m[1][0]=Xy; r.m[1][1]=Yy; r.m[1][2]=Oy;
        return r;
    }

    // Проективна система координат: точки X, Y, O з вагами wx, wy, w0
    // (однорідні координати вагових точок):
    //   x' = (Xx*wx*x + Yx*wy*y + Ox*w0) / (wx*x + wy*y + w0)
    //   y' = (Xy*wx*x + Yy*wy*y + Oy*w0) / (wx*x + wy*y + w0)
    static Matrix3 projective(double Xx,double Xy,double wx,
                              double Yx,double Yy,double wy,
                              double Ox,double Oy,double w0) {
        Matrix3 r{};
        r.m[0][0]=Xx*wx; r.m[0][1]=Yx*wy; r.m[0][2]=Ox*w0;
        r.m[1][0]=Xy*wx; r.m[1][1]=Yy*wy; r.m[1][2]=Oy*w0;
        r.m[2][0]=wx;    r.m[2][1]=wy;    r.m[2][2]=w0;
        return r;
    }

    // Відображення з перевіркою. Повертає false, якщо точка в нескінченності
    // або лежить "за камерою" (знак W не збігається з refSign).
    bool mapChecked(const QPointF& p, QPointF& out, double refSign = 1.0) const {
        const double x=p.x(), y=p.y();
        const double X=m[0][0]*x+m[0][1]*y+m[0][2];
        const double Y=m[1][0]*x+m[1][1]*y+m[1][2];
        const double W=m[2][0]*x+m[2][1]*y+m[2][2];
        if(W*refSign < 1e-9) return false;
        out = QPointF(X/W, Y/W);
        return std::isfinite(out.x()) && std::isfinite(out.y());
    }

    QPointF map(const QPointF& p) const {
        QPointF r;
        if(mapChecked(p, r, (m[2][2] < 0.0) ? -1.0 : 1.0)) return r;
        return QPointF(NAN, NAN);
    }

    double det() const {
        return m[0][0]*(m[1][1]*m[2][2]-m[1][2]*m[2][1])
             - m[0][1]*(m[1][0]*m[2][2]-m[1][2]*m[2][0])
             + m[0][2]*(m[1][0]*m[2][1]-m[1][1]*m[2][0]);
    }

    // Обернена матриця (через алгебраїчні доповнення). ok=false, якщо det≈0.
    Matrix3 inverse(bool* ok = nullptr) const {
        const double d = det();
        Matrix3 r{};
        if(std::abs(d) < 1e-12) { if(ok) *ok=false; return identity(); }
        r.m[0][0] =  (m[1][1]*m[2][2]-m[1][2]*m[2][1])/d;
        r.m[0][1] = -(m[0][1]*m[2][2]-m[0][2]*m[2][1])/d;
        r.m[0][2] =  (m[0][1]*m[1][2]-m[0][2]*m[1][1])/d;
        r.m[1][0] = -(m[1][0]*m[2][2]-m[1][2]*m[2][0])/d;
        r.m[1][1] =  (m[0][0]*m[2][2]-m[0][2]*m[2][0])/d;
        r.m[1][2] = -(m[0][0]*m[1][2]-m[0][2]*m[1][0])/d;
        r.m[2][0] =  (m[1][0]*m[2][1]-m[1][1]*m[2][0])/d;
        r.m[2][1] = -(m[0][0]*m[2][1]-m[0][1]*m[2][0])/d;
        r.m[2][2] =  (m[0][0]*m[1][1]-m[0][1]*m[1][0])/d;
        if(ok) *ok=true;
        return r;
    }

    Matrix3 operator*(const Matrix3& b) const {
        Matrix3 r{};
        for(int i=0;i<3;++i)
            for(int j=0;j<3;++j)
                for(int k=0;k<3;++k)
                    r.m[i][j]+=m[i][k]*b.m[k][j];
        return r;
    }
};
