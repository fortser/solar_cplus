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
#include <QGraphicsSimpleTextItem> // Для текста на карте
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
    
    void saveSimulation();
    void loadSimulation();
    
    // Слот для отслеживания наведения мыши (опционально можно расширить)
    void onSelectionChanged();

private:
    PhysicsEngine physics;
    
    QGraphicsScene* scene;
    InteractiveView* view;
    QTimer* timer;

    // Кнопки
    QPushButton* btnPlayPause;
    QPushButton* btnReset;
    QPushButton* btnSave; 
    QPushButton* btnLoad; 
    // Новые кнопки зума
    QPushButton* btnZoomIn;
    QPushButton* btnZoomOut;

    QSlider* sliderSpeed;
    QLabel* labelSpeed;
    
    // Новая метка для инфо
    QLabel* labelInfo;

    double scaleFactor = 100.0 / 1.496e11; 
    double baseTimeStep = 3600 * 24;       
    double currentSpeedMultiplier = 1.0;   

    std::vector<QGraphicsEllipseItem*> bodyItems;
    // Вектор для текстовых подписей (имен планет)
    std::vector<QGraphicsSimpleTextItem*> nameLabels;

    void setupSystem();    
    void clearSystem();    
    void createVisuals();  
    void drawBodies();     
};