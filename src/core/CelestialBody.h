#pragma once
#include <QString>
#include <QColor>
#include <Eigen/Dense>

struct CelestialBody {
    QString name;
    double mass;       // Масса в кг
    double radius;     // Радиус тела (для отрисовки)
    QColor color;

    // Векторы состояния (используем 2D для MVP)
    Eigen::Vector2d position;     // Позиция (м)
    Eigen::Vector2d velocity;     // Скорость (м/с)
    Eigen::Vector2d acceleration; // Ускорение (м/с^2)

    CelestialBody(QString n, double m, double r, QColor c, Eigen::Vector2d pos, Eigen::Vector2d vel)
        : name(n), mass(m), radius(r), color(c), position(pos), velocity(vel) {
        acceleration.setZero();
    }
};