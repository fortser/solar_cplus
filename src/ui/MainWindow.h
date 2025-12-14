#pragma once
#include <QMainWindow>
#include <QTimer>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QDockWidget>
#include <QTextEdit>

// Qt 3D
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DExtras/QOrbitCameraController>
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DRender/QPointLight>
#include <Qt3DRender/QObjectPicker> // <-- Используем это для кликов
#include <Qt3DRender/QPickEvent>

#include "../core/PhysicsEngine.h"

struct VisualBody3D {
    Qt3DCore::QEntity* entity;
    Qt3DCore::QTransform* transform;
    int physicsIndex;
};

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
    void onIntegratorChanged(int index);
    void onRelativityToggled(bool checked);

    // Управление камерой
    void zoomIn();
    void zoomOut();
    void resetCamera();

    // НОВЫЙ СЛОТ ДЛЯ КЛИКОВ (Простой и надежный)
    void onObjectPicked(Qt3DRender::QPickEvent* event);

private:
    PhysicsEngine physics;
    QTimer* timer;

    Qt3DExtras::Qt3DWindow* view3D;
    Qt3DCore::QEntity* rootEntity;
    Qt3DExtras::QOrbitCameraController* cameraController;

    std::vector<VisualBody3D> visualBodies;
    int selectedBodyIndex = -1;

    QPushButton *btnPlayPause, *btnReset, *btnSave, *btnLoad;
    QPushButton *btnZoomIn, *btnZoomOut, *btnResetView;
    QSlider* sliderSpeed;
    QLabel* labelSpeed;
    QComboBox* comboIntegrator;
    QCheckBox* checkRelativity;
    
    QDockWidget* infoDock;
    QTextEdit* infoText;

    double scaleFactor = 100.0 / 1.496e11;
    double baseTimeStep = 3600 * 24;
    double currentSpeedMultiplier = 1.0;

    void setupScene();
    void setupSystem();
    void clearSystem();
    void createVisuals();
    void updateVisuals();
    void updateInfoPanel();
};