#pragma once
#include <vector>
#include <cmath>
#include <omp.h>
#include "CelestialBody.h"

enum class IntegratorType {
    Verlet,
    RungeKutta4
};

class PhysicsEngine {
public:
    const double G = 6.67430e-11;
    const double C = 299792458.0; 

    std::vector<CelestialBody> bodies;
    
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
    struct State {
        Eigen::Vector3d pos;
        Eigen::Vector3d vel;
    };

    // --- БУФЕРЫ ПАМЯТИ ---
    // Разделяем буферы, чтобы старые данные не перезаписывались новыми
    std::vector<Eigen::Vector3d> m_accBuffer;     // Для расчета текущих сил
    std::vector<Eigen::Vector3d> m_oldAccBuffer;  // Специально для Verlet (хранит a(t))
    
    // Буферы для RK4
    std::vector<State> m_stateBuffer;         
    std::vector<Eigen::Vector3d> m_kAccBuffer;

    // --- Velocity Verlet (Стабильный) ---
    void stepVerlet(double dt) {
        int n = (int)bodies.size();
        
        // 1. r(t+dt) = r(t) + v(t)dt + 0.5 * a(t) * dt^2
        #pragma omp parallel for
        for (int i = 0; i < n; ++i) {
            bodies[i].position += bodies[i].velocity * dt + 0.5 * bodies[i].acceleration * dt * dt;
        }

        // 2. Сохраняем a(t) в отдельный буфер перед пересчетом
        if (m_oldAccBuffer.size() != n) m_oldAccBuffer.resize(n);
        
        #pragma omp parallel for
        for (int i = 0; i < n; ++i) m_oldAccBuffer[i] = bodies[i].acceleration;

        // 3. Считаем a(t+dt). 
        // ВНИМАНИЕ: Это пишет в m_accBuffer, но не трогает m_oldAccBuffer!
        computeAccelerationsForState(bodies); 

        // 4. v(t+dt) = v(t) + 0.5 * (a(t) + a(t+dt)) * dt
        #pragma omp parallel for
        for (int i = 0; i < n; ++i) {
            bodies[i].velocity += 0.5 * (m_oldAccBuffer[i] + bodies[i].acceleration) * dt;
        }
    }

    // --- Runge-Kutta 4 (Точный) ---
    void stepRK4(double dt) {
        int n = (int)bodies.size();
        if (m_stateBuffer.size() != n) m_stateBuffer.resize(n);
        
        // Начальное состояние
        #pragma omp parallel for
        for(int i=0; i<n; ++i) m_stateBuffer[i] = {bodies[i].position, bodies[i].velocity};

        // K1
        computeAccFromState(m_stateBuffer, m_kAccBuffer); // k1_acc
        std::vector<Eigen::Vector3d> k1_v = m_kAccBuffer; 
        std::vector<Eigen::Vector3d> k1_x(n);
        for(int i=0; i<n; ++i) k1_x[i] = bodies[i].velocity;

        // K2
        #pragma omp parallel for
        for(int i=0; i<n; ++i) {
            m_stateBuffer[i].pos = bodies[i].position + k1_x[i] * (dt / 2.0);
            m_stateBuffer[i].vel = bodies[i].velocity + k1_v[i] * (dt / 2.0);
        }
        computeAccFromState(m_stateBuffer, m_kAccBuffer); // k2_acc
        std::vector<Eigen::Vector3d> k2_v = m_kAccBuffer;
        std::vector<Eigen::Vector3d> k2_x(n);
        for(int i=0; i<n; ++i) k2_x[i] = bodies[i].velocity + k1_v[i] * (dt / 2.0);

        // K3
        #pragma omp parallel for
        for(int i=0; i<n; ++i) {
            m_stateBuffer[i].pos = bodies[i].position + k2_x[i] * (dt / 2.0);
            m_stateBuffer[i].vel = bodies[i].velocity + k2_v[i] * (dt / 2.0);
        }
        computeAccFromState(m_stateBuffer, m_kAccBuffer); // k3_acc
        std::vector<Eigen::Vector3d> k3_v = m_kAccBuffer;
        std::vector<Eigen::Vector3d> k3_x(n);
        for(int i=0; i<n; ++i) k3_x[i] = bodies[i].velocity + k2_v[i] * (dt / 2.0);

        // K4
        #pragma omp parallel for
        for(int i=0; i<n; ++i) {
            m_stateBuffer[i].pos = bodies[i].position + k3_x[i] * dt;
            m_stateBuffer[i].vel = bodies[i].velocity + k3_v[i] * dt;
        }
        computeAccFromState(m_stateBuffer, m_kAccBuffer); // k4_acc
        std::vector<Eigen::Vector3d> k4_v = m_kAccBuffer;
        std::vector<Eigen::Vector3d> k4_x(n);
        for(int i=0; i<n; ++i) k4_x[i] = bodies[i].velocity + k3_v[i] * dt;

        // Финал
        #pragma omp parallel for
        for(int i=0; i<n; ++i) {
            bodies[i].position += (dt / 6.0) * (k1_x[i] + 2.0*k2_x[i] + 2.0*k3_x[i] + k4_x[i]);
            bodies[i].velocity += (dt / 6.0) * (k1_v[i] + 2.0*k2_v[i] + 2.0*k3_v[i] + k4_v[i]);
        }
        
        // Обновляем ускорение для следующего шага
        computeAccelerationsForState(bodies);
    }

    // Расчет сил (OpenMP)
    void computeAccFromState(const std::vector<State>& states, std::vector<Eigen::Vector3d>& results) {
        int n = (int)states.size();
        if (results.size() != n) results.resize(n);

        #pragma omp parallel for schedule(dynamic)
        for (int i = 0; i < n; ++i) {
            Eigen::Vector3d currentAcc(0, 0, 0);
            
            for (int j = 0; j < n; ++j) {
                if (i == j) continue; 

                Eigen::Vector3d r_vec = states[j].pos - states[i].pos;
                double dist2 = r_vec.squaredNorm(); 
                
                // Защита от столкновений (мягкое ядро)
                if (dist2 < 1e10) continue; 

                double dist = std::sqrt(dist2);
                double dist3 = dist2 * dist;
                
                Eigen::Vector3d a_newton = r_vec * (G * bodies[j].mass / dist3);
                
                if (useRelativity) {
                    double v_sq = states[i].vel.squaredNorm(); 
                    double correction = 1.0 + (3.0 * v_sq) / (C * C); 
                    currentAcc += a_newton * correction;
                } else {
                    currentAcc += a_newton;
                }
            }
            results[i] = currentAcc;
        }
    }

    void computeAccelerationsForState(std::vector<CelestialBody>& currentBodies) {
        int n = (int)currentBodies.size();
        if (m_stateBuffer.size() != n) m_stateBuffer.resize(n);
        
        #pragma omp parallel for
        for(int i=0; i<n; ++i) {
            m_stateBuffer[i] = {currentBodies[i].position, currentBodies[i].velocity};
        }
        
        // Используем m_accBuffer как временное хранилище
        if (m_accBuffer.size() != n) m_accBuffer.resize(n);
        computeAccFromState(m_stateBuffer, m_accBuffer);
        
        #pragma omp parallel for
        for(int i=0; i<n; ++i) {
            currentBodies[i].acceleration = m_accBuffer[i];
        }
    }
};