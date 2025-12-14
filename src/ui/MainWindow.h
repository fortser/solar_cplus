#pragma once
#include <QMainWindow>
#include <QGraphicsScene>
#include <QTimer>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QHBoxLayout>
#include <QComboBox> // <-- Новый
#include <QCheckBox> // <-- Новый
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QGraphicsSimpleTextItem>
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
    void onSelectionChanged();
    
    // Новые слоты настроек
    void onIntegratorChanged(int index);
    void onRelativityToggled(bool checked);

private:
    PhysicsEngine physics;
    
    QGraphicsScene* scene;
    InteractiveView* view;
    QTimer* timer;

    QPushButton* btnPlayPause;
    QPushButton* btnReset;
    QPushButton* btnSave; 
    QPushButton* btnLoad; 
    QPushButton* btnZoomIn;
    QPushButton* btnZoomOut;

    QSlider* sliderSpeed;
    QLabel* labelSpeed;
    QLabel* labelInfo;
    
    // Новые контролы
    QComboBox* comboIntegrator;
    QCheckBox* checkRelativity;

    double scaleFactor = 100.0 / 1.496e11; 
    double baseTimeStep = 3600 * 24;       
    double currentSpeedMultiplier = 1.0;   

    std::vector<QGraphicsEllipseItem*> bodyItems;
    std::vector<QGraphicsSimpleTextItem*> nameLabels;

    void setupSystem();    
    void clearSystem();    
    void createVisuals();  
    void drawBodies();     
};