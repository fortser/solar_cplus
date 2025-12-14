#pragma once
#include <QString>
#include <QColor>
#include <Eigen/Dense>

struct CelestialBody {
    QString name;
    double mass;       
    double radius;     
    QColor color;

    // Векторы состояния теперь 3D (x, y, z)
    Eigen::Vector3d position;     
    Eigen::Vector3d velocity;     
    Eigen::Vector3d acceleration; 

    CelestialBody(QString n, double m, double r, QColor c, Eigen::Vector3d pos, Eigen::Vector3d vel)
        : name(n), mass(m), radius(r), color(c), position(pos), velocity(vel) {
        acceleration.setZero();
    }
};