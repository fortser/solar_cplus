#pragma once
#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTimer>
#include "../core/PhysicsEngine.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void updateSimulation(); // Слот, вызываемый таймером

private:
    PhysicsEngine physics;
    
    QGraphicsScene* scene;
    QGraphicsView* view;
    QTimer* timer;

    // Масштаб: сколько пикселей в 1 метре.
    // Солнечная система огромна, 1 АЕ ≈ 1.5e11 метров.
    // Пусть 100 пикселей = 1 АЕ.
    double scaleFactor = 100.0 / 1.496e11; 

    // Визуальные элементы (кружочки), соответствующие телам
    std::vector<QGraphicsEllipseItem*> bodyItems;

    void setupSystem(); // Создаем Солнце и Землю
    void drawBodies();  // Обновляем положение кружочков
};