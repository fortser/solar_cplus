#pragma once
#include <vector>
#include <cmath>
#include "CelestialBody.h"

class PhysicsEngine {
public:
    // Гравитационная постоянная
    const double G = 6.67430e-11;

    std::vector<CelestialBody> bodies;

    void addBody(const CelestialBody& body) {
        bodies.push_back(body);
    }

    // Основной шаг симуляции (метод Velocity Verlet)
    void step(double dt) {
        // 1. Обновляем позицию: r(t+dt) = r(t) + v(t)dt + 0.5*a(t)dt^2
        for (auto& body : bodies) {
            body.position += body.velocity * dt + 0.5 * body.acceleration * dt * dt;
        }

        // 2. Сохраняем старое ускорение и вычисляем новое
        // (Для Verlet нам нужно a(t) и a(t+dt))
        std::vector<Eigen::Vector2d> old_accelerations;
        for (const auto& body : bodies) {
            old_accelerations.push_back(body.acceleration);
        }

        computeForces(); // Вычисляем a(t+dt)

        // 3. Обновляем скорость: v(t+dt) = v(t) + 0.5 * (a(t) + a(t+dt)) * dt
        for (size_t i = 0; i < bodies.size(); ++i) {
            bodies[i].velocity += 0.5 * (old_accelerations[i] + bodies[i].acceleration) * dt;
        }
    }

private:
    void computeForces() {
        // Обнуляем ускорение перед расчетом
        for (auto& body : bodies) {
            body.acceleration.setZero();
        }

        // Закон всемирного тяготения: F = G * m1 * m2 / r^2
        for (size_t i = 0; i < bodies.size(); ++i) {
            for (size_t j = i + 1; j < bodies.size(); ++j) {
                Eigen::Vector2d r_vec = bodies[j].position - bodies[i].position;
                double dist = r_vec.norm();
                
                // Избегаем деления на ноль (коллизии игнорируем в MVP)
                if (dist < 1e3) continue; 

                double f_mag = (G * bodies[i].mass * bodies[j].mass) / (dist * dist);
                
                Eigen::Vector2d f_vec = r_vec.normalized() * f_mag;

                // F = ma => a = F/m
                bodies[i].acceleration += f_vec / bodies[i].mass;
                bodies[j].acceleration -= f_vec / bodies[j].mass; // Третий закон Ньютона
            }
        }
    }
};