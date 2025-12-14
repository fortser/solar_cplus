#include <gtest/gtest.h>
#include "../src/core/PhysicsEngine.h"
#include <cmath>

// Тест 1: Проверка формулы гравитации
TEST(PhysicsTest, GravitationalForceCalculation) {
    PhysicsEngine physics;
    
    double m1 = 1.0e5; // 100 тонн
    double m2 = 2.0e5; // 200 тонн
    double dist = 2000.0; // 2000 метров (больше лимита в 1000м)
    
    physics.addBody(CelestialBody("Obj1", m1, 1, Qt::white, {0, 0}, {0, 0}));
    physics.addBody(CelestialBody("Obj2", m2, 1, Qt::white, {dist, 0}, {0, 0}));
    
    // Делаем микро-шаг
    physics.step(1e-9); 
    
    // Считаем ожидаемую силу вручную: F = G * m1 * m2 / r^2
    const double G = 6.67430e-11;
    double expectedForce = G * (m1 * m2) / (dist * dist);
    
    // Ожидаемое ускорение первого тела: a = F / m1
    double expectedAcc1 = expectedForce / m1;
    
    // Получаем реальное ускорение
    double actualAcc1 = physics.bodies[0].acceleration.norm();
    
    // Сравниваем
    EXPECT_NEAR(actualAcc1, expectedAcc1, 1e-15);
}

// Тест 2: Проверка Интегратора (двигается ли тело?)
TEST(PhysicsTest, IntegrationMovement) {
    PhysicsEngine physics;
    
    // Тело летит со скоростью 10 м/с вправо
    physics.addBody(CelestialBody("Runner", 10, 1, Qt::white, {0, 0}, {10, 0}));
    
    // Шагаем 1 секунду
    physics.step(1.0);
    
    // Новая позиция должна быть (10, 0)
    EXPECT_NEAR(physics.bodies[0].position.x(), 10.0, 1e-9);
    EXPECT_NEAR(physics.bodies[0].position.y(), 0.0, 1e-9);
}