#pragma once
#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTimer>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QHBoxLayout>
#include "../core/PhysicsEngine.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void updateSimulation();      // Вызывается таймером
    void toggleSimulation();      // Кнопка Старт/Пауза
    void resetSimulation();       // Кнопка Сброс
    void onSpeedChanged(int val); // Изменение слайдера

private:
    PhysicsEngine physics;
    
    QGraphicsScene* scene;
    QGraphicsView* view;
    QTimer* timer;

    // Элементы управления
    QPushButton* btnPlayPause;
    QPushButton* btnReset;
    QSlider* sliderSpeed;
    QLabel* labelSpeed;

    // Параметры симуляции
    double scaleFactor = 100.0 / 1.496e11; // 100 пикс = 1 АЕ
    double baseTimeStep = 3600 * 24;       // 1 день (секунд)
    double currentSpeedMultiplier = 1.0;   // Множитель скорости

    // Визуальные элементы
    std::vector<QGraphicsEllipseItem*> bodyItems;

    void setupSystem();    // Создание тел
    void clearSystem();    // Очистка перед сбросом
    void drawBodies();     // Обновление графики
};