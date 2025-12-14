#pragma once
#include <QMainWindow>
#include <QGraphicsScene>
#include <QTimer>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include "../core/PhysicsEngine.h"
#include "InteractiveView.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void updateSimulation();
    void toggleSimulation();
    void resetSimulation();
    void onSpeedChanged(int val);
    
    // Новые слоты для файлов
    void saveSimulation();
    void loadSimulation();

private:
    PhysicsEngine physics;
    
    QGraphicsScene* scene;
    InteractiveView* view;
    QTimer* timer;

    QPushButton* btnPlayPause;
    QPushButton* btnReset;
    QPushButton* btnSave; // Новая кнопка
    QPushButton* btnLoad; // Новая кнопка
    QSlider* sliderSpeed;
    QLabel* labelSpeed;

    double scaleFactor = 100.0 / 1.496e11; 
    double baseTimeStep = 3600 * 24;       
    double currentSpeedMultiplier = 1.0;   

    std::vector<QGraphicsEllipseItem*> bodyItems;

    void setupSystem();    // Создание стандартной системы (Земля/Марс)
    void clearSystem();    // Очистка
    void createVisuals();  // Создание графики для тел, которые уже есть в physics
    void drawBodies();     // Обновление позиций
};