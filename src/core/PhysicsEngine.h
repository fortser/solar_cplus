#pragma once
#include <vector>
#include <cmath>
#include "CelestialBody.h"

enum class IntegratorType {
    Verlet,
    RungeKutta4
};

class PhysicsEngine {
public:
    const double G = 6.67430e-11;
    const double C = 299792458.0; // Скорость света

    std::vector<CelestialBody> bodies;
    
    // Настройки
    IntegratorType currentIntegrator = IntegratorType::Verlet;
    bool useRelativity = false;

    void addBody(const CelestialBody& body) {
        bodies.push_back(body);
    }

    void step(double dt) {
        if (currentIntegrator == IntegratorType::Verlet) {
            stepVerlet(dt);
        } else {
            stepRK4(dt);
        }
    }

private:
    // --- Метод Верле (Velocity Verlet) ---
    void stepVerlet(double dt) {
        // 1. Позиция
        for (auto& body : bodies) {
            body.position += body.velocity * dt + 0.5 * body.acceleration * dt * dt;
        }

        // 2. Сохраняем старое ускорение
        std::vector<Eigen::Vector3d> old_acc;
        for (const auto& body : bodies) old_acc.push_back(body.acceleration);

        // 3. Новое ускорение
        computeAccelerationsForState(bodies); 

        // 4. Скорость
        for (size_t i = 0; i < bodies.size(); ++i) {
            bodies[i].velocity += 0.5 * (old_acc[i] + bodies[i].acceleration) * dt;
        }
    }

    // --- Метод Рунге-Кутты 4 (RK4) ---
    struct State {
        Eigen::Vector3d pos;
        Eigen::Vector3d vel;
    };

    void stepRK4(double dt) {
        size_t n = bodies.size();
        std::vector<State> y0(n); // Начальное состояние
        
        for(size_t i=0; i<n; ++i) {
            y0[i] = {bodies[i].position, bodies[i].velocity};
        }

        // k1
        auto k1_acc = getAccFromState(y0);
        std::vector<State> y1(n);
        for(size_t i=0; i<n; ++i) {
            y1[i].pos = y0[i].pos + y0[i].vel * (dt / 2.0);
            y1[i].vel = y0[i].vel + k1_acc[i] * (dt / 2.0);
        }

        // k2
        auto k2_acc = getAccFromState(y1);
        std::vector<State> y2(n);
        for(size_t i=0; i<n; ++i) {
            y2[i].pos = y0[i].pos + y1[i].vel * (dt / 2.0);
            y2[i].vel = y0[i].vel + k2_acc[i] * (dt / 2.0);
        }

        // k3
        auto k3_acc = getAccFromState(y2);
        std::vector<State> y3(n);
        for(size_t i=0; i<n; ++i) {
            y3[i].pos = y0[i].pos + y2[i].vel * dt;
            y3[i].vel = y0[i].vel + k3_acc[i] * dt;
        }

        // k4
        auto k4_acc = getAccFromState(y3);

        // Итоговое обновление
        for(size_t i=0; i<n; ++i) {
            bodies[i].position += (dt / 6.0) * (y0[i].vel + 2.0*y1[i].vel + 2.0*y2[i].vel + y3[i].vel);
            bodies[i].velocity += (dt / 6.0) * (k1_acc[i] + 2.0*k2_acc[i] + 2.0*k3_acc[i] + k4_acc[i]);
        }
        
        // Обновляем ускорение для отображения/логики
        computeAccelerationsForState(bodies);
    }

    // Вспомогательная функция для RK4: расчет ускорений для гипотетического состояния
    std::vector<Eigen::Vector3d> getAccFromState(const std::vector<State>& states) {
        size_t n = states.size();
        std::vector<Eigen::Vector3d> accs(n, Eigen::Vector3d::Zero());

        for (size_t i = 0; i < n; ++i) {
            for (size_t j = i + 1; j < n; ++j) {
                Eigen::Vector3d r_vec = states[j].pos - states[i].pos;
                double dist = r_vec.norm();
                if (dist < 1e3) continue;

                double dist2 = dist * dist;
                double dist3 = dist2 * dist;
                
                // Ньютоновская сила F/m = a
                Eigen::Vector3d a_newton = r_vec * (G / dist3);
                
                // --- Релятивистская поправка (Post-Newtonian) ---
                // Упрощенная модель для прецессии: a = a_newton * (1 + alpha * v^2 / c^2 ...)
                double correction = 1.0;
                if (useRelativity) {
                    // Очень упрощенный член 1PN (достаточен для эффекта прецессии)
                    double v_sq = states[i].vel.squaredNorm(); 
                    correction = 1.0 + (3.0 * v_sq) / (C * C); 
                }

                accs[i] += a_newton * bodies[j].mass * correction;
                accs[j] -= a_newton * bodies[i].mass * correction; // Newton's 3rd law (approx)
            }
        }
        return accs;
    }

    // Расчет ускорений для текущих реальных тел (для Verlet)
    void computeAccelerationsForState(std::vector<CelestialBody>& currentBodies) {
        // Конвертируем bodies в стейт и используем общую логику
        std::vector<State> states(currentBodies.size());
        for(size_t i=0; i<currentBodies.size(); ++i) {
            states[i] = {currentBodies[i].position, currentBodies[i].velocity};
        }
        auto accs = getAccFromState(states);
        for(size_t i=0; i<currentBodies.size(); ++i) {
            currentBodies[i].acceleration = accs[i];
        }
    }
};