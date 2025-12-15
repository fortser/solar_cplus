#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QQuaternion> 

#include <Qt3DExtras/QForwardRenderer>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QCameraLens>
#include <QFont>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    // 1. 3D Window
    view3D = new Qt3DExtras::Qt3DWindow();
    view3D->defaultFrameGraph()->setClearColor(QColor(Qt::black));
    
    QWidget* container3D = QWidget::createWindowContainer(view3D);
    container3D->setMinimumSize(QSize(800, 600));

    rootEntity = new Qt3DCore::QEntity();
    view3D->setRootEntity(rootEntity);

    // 2. Info Panel
    infoDock = new QDockWidget("Object Inspector", this);
    infoDock->setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea);
    infoText = new QTextEdit();
    infoText->setReadOnly(true);
    infoText->setStyleSheet("background-color: #2b2b2b; color: #f0f0f0; font-family: Consolas; font-size: 12px;");
    infoDock->setWidget(infoText);
    addDockWidget(Qt::RightDockWidgetArea, infoDock);

    // 3. UI
    auto centralWidget = new QWidget(this);
    auto mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->addWidget(container3D, 1);

    auto controlsLayout = new QHBoxLayout();
    
    btnPlayPause = new QPushButton("Pause", this);
    connect(btnPlayPause, &QPushButton::clicked, this, &MainWindow::toggleSimulation);
    controlsLayout->addWidget(btnPlayPause);

    btnReset = new QPushButton("Reset Logic", this);
    connect(btnReset, &QPushButton::clicked, this, &MainWindow::resetSimulation);
    controlsLayout->addWidget(btnReset);

    btnSave = new QPushButton("Save", this);
    connect(btnSave, &QPushButton::clicked, this, &MainWindow::saveSimulation);
    controlsLayout->addWidget(btnSave);

    btnLoad = new QPushButton("Load", this);
    connect(btnLoad, &QPushButton::clicked, this, &MainWindow::loadSimulation);
    controlsLayout->addWidget(btnLoad);

    controlsLayout->addSpacing(15);

    btnZoomIn = new QPushButton("(+)", this);
    btnZoomIn->setFixedWidth(30);
    connect(btnZoomIn, &QPushButton::clicked, this, &MainWindow::zoomIn);
    controlsLayout->addWidget(btnZoomIn);

    btnZoomOut = new QPushButton("(-)", this);
    btnZoomOut->setFixedWidth(30);
    connect(btnZoomOut, &QPushButton::clicked, this, &MainWindow::zoomOut);
    controlsLayout->addWidget(btnZoomOut);

    btnResetView = new QPushButton("Reset View", this);
    connect(btnResetView, &QPushButton::clicked, this, &MainWindow::resetCamera);
    controlsLayout->addWidget(btnResetView);

    controlsLayout->addSpacing(15);

    auto physicsLayout = new QVBoxLayout();
    
    auto visualSettingsLayout = new QHBoxLayout();
    checkShowLabels = new QCheckBox("Labels", this);
    checkShowLabels->setChecked(true);
    connect(checkShowLabels, &QCheckBox::toggled, this, &MainWindow::onShowLabelsToggled);
    visualSettingsLayout->addWidget(checkShowLabels);

    checkShowTrails = new QCheckBox("Trails", this);
    checkShowTrails->setChecked(true);
    connect(checkShowTrails, &QCheckBox::toggled, this, &MainWindow::onShowTrailsToggled);
    visualSettingsLayout->addWidget(checkShowTrails);
    physicsLayout->addLayout(visualSettingsLayout);

    comboIntegrator = new QComboBox(this);
    comboIntegrator->addItem("Verlet (Fast)");
    comboIntegrator->addItem("Runge-Kutta 4 (Precise)");
    connect(comboIntegrator, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onIntegratorChanged);
    physicsLayout->addWidget(comboIntegrator);

    checkRelativity = new QCheckBox("Gen. Relativity", this);
    connect(checkRelativity, &QCheckBox::toggled, this, &MainWindow::onRelativityToggled);
    physicsLayout->addWidget(checkRelativity);
    controlsLayout->addLayout(physicsLayout);

    controlsLayout->addSpacing(15);

    controlsLayout->addWidget(new QLabel("Speed:", this));
    sliderSpeed = new QSlider(Qt::Horizontal, this);
    sliderSpeed->setRange(0, 500); sliderSpeed->setValue(100);
    connect(sliderSpeed, &QSlider::valueChanged, this, &MainWindow::onSpeedChanged);
    controlsLayout->addWidget(sliderSpeed);
    labelSpeed = new QLabel("1.0x", this);
    labelSpeed->setMinimumWidth(50);
    controlsLayout->addWidget(labelSpeed);

    mainLayout->addLayout(controlsLayout); 
    setCentralWidget(centralWidget);
    resize(1400, 850);
    setWindowTitle("Solar Simulator v2.9 - Memory Safe");

    setupScene();
    setupSystem();

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateSimulation);
    timer->start(16); 
}

void MainWindow::setupScene() {
    auto camera = view3D->camera();
    camera->lens()->setPerspectiveProjection(45.0f, 16.0f/9.0f, 0.1f, 100000.0f);
    resetCamera();

    cameraController = new Qt3DExtras::QOrbitCameraController(rootEntity);
    cameraController->setCamera(camera);
    cameraController->setLinearSpeed(300.0f);
    cameraController->setLookSpeed(180.0f);

    auto lightEntity = new Qt3DCore::QEntity(rootEntity);
    auto pointLight = new Qt3DRender::QPointLight(lightEntity);
    pointLight->setColor(Qt::white);
    pointLight->setIntensity(1.2f);
    lightEntity->addComponent(pointLight);

    orbitGrid = new OrbitGrid(rootEntity, scaleFactor);
}

void MainWindow::createVisuals() {
    for (size_t i = 0; i < physics.bodies.size(); ++i) {
        auto& body = physics.bodies[i];
        VisualBody3D vb;
        vb.physicsIndex = i;
        vb.entity = new Qt3DCore::QEntity(rootEntity);
        vb.entity->setObjectName(QString::number(i));

        auto mesh = new Qt3DExtras::QSphereMesh();
        double r = 3.0;
        if (body.name == "Sun") r = 20.0;
        else if (body.name == "Jupiter") r = 10.0;
        else if (body.name == "Saturn") r = 9.0;
        else if (body.name == "Earth") r = 5.0;
        else if (body.name == "Halley's Comet") r = 2.0;
        mesh->setRadius(r);
        mesh->setRings(30); mesh->setSlices(30);

        vb.transform = new Qt3DCore::QTransform();
        auto mat = new Qt3DExtras::QPhongMaterial();
        mat->setDiffuse(body.color);
        if (body.name == "Sun") mat->setAmbient(body.color);
        else { mat->setAmbient(QColor(60, 60, 60)); mat->setShininess(10.0f); }

        vb.entity->addComponent(mesh);
        vb.entity->addComponent(vb.transform);
        vb.entity->addComponent(mat);

        auto picker = new Qt3DRender::QObjectPicker(vb.entity);
        picker->setHoverEnabled(false);
        connect(picker, &Qt3DRender::QObjectPicker::clicked, this, &MainWindow::onObjectPicked);
        vb.entity->addComponent(picker);

        if (body.name != "Sun") {
            vb.trail = new OrbitTrail(rootEntity, body.color, 2000); 
            vb.trail->setEnabled(checkShowTrails->isChecked());
        } else {
            vb.trail = nullptr;
        }

        vb.label = new Qt3DExtras::QText2DEntity(rootEntity);
        vb.label->setText(body.name);
        vb.label->setHeight(20); 
        vb.label->setWidth(100); 
        vb.label->setColor(Qt::white);
        vb.label->setFont(QFont("Arial", 10, QFont::Bold));
        vb.label->setEnabled(checkShowLabels->isChecked());
        
        vb.labelTransform = new Qt3DCore::QTransform();
        vb.label->addComponent(vb.labelTransform);

        visualBodies.push_back(vb);
    }
    updateVisuals();
}

void MainWindow::onObjectPicked(Qt3DRender::QPickEvent* event) {
    auto entity = qobject_cast<Qt3DCore::QEntity*>(sender()->parent());
    if (entity) {
        bool ok;
        int idx = entity->objectName().toInt(&ok);
        if (ok) {
            selectedBodyIndex = idx;
            updateInfoPanel();
        }
    }
}

void MainWindow::updateInfoPanel() {
    if (selectedBodyIndex == -1) {
        infoText->setHtml("<div style='text-align:center; margin-top:20px; color:#888;'><i>Click on a planet</i></div>");
        return;
    }
    auto& b = physics.bodies[selectedBodyIndex];
    QString html = QString("<h2 style='color:%1'>%2</h2>").arg(b.color.name(), b.name);
    html += "<table width='100%'>";
    html += QString("<tr><td>Mass:</td><td>%1 kg</td></tr>").arg(b.mass, 0, 'e', 2);
    html += QString("<tr><td>Speed:</td><td>%1 km/s</td></tr>").arg(b.velocity.norm()/1000.0, 0, 'f', 2);
    html += QString("<tr><td>Dist:</td><td>%1 AU</td></tr>").arg(b.position.norm()/1.496e11, 0, 'f', 3);
    html += "</table>";
    infoText->setHtml(html);
}

void MainWindow::updateVisuals() {
    trailSkipCounter++;
    bool updateTrail = (trailSkipCounter >= 3);

    QVector3D cameraPos = view3D->camera()->position();

    for (size_t i = 0; i < visualBodies.size(); ++i) {
        auto p = physics.bodies[visualBodies[i].physicsIndex].position;
        
        float x = (float)(p.x() * scaleFactor);
        float y = (float)(p.z() * scaleFactor);
        float z = (float)(p.y() * scaleFactor);
        QVector3D pos3D(x, y, z);

        visualBodies[i].transform->setTranslation(pos3D);

        if (visualBodies[i].labelTransform) {
            visualBodies[i].labelTransform->setTranslation(QVector3D(x + 5, y + 10, z));
            QVector3D direction = cameraPos - pos3D;
            visualBodies[i].labelTransform->setRotation(QQuaternion::fromDirection(direction, QVector3D(0, 1, 0)));
        }

        if (updateTrail && visualBodies[i].trail && visualBodies[i].trail->isEnabled()) {
            visualBodies[i].trail->update(pos3D);
        }
    }

    if (updateTrail) trailSkipCounter = 0;
}

void MainWindow::updateSimulation() {
    physics.step(baseTimeStep * currentSpeedMultiplier);
    updateVisuals();
    if (selectedBodyIndex != -1) updateInfoPanel();
}

// --- ИСПРАВЛЕННАЯ ФУНКЦИЯ ОЧИСТКИ (MEMORY SAFE) ---
void MainWindow::clearSystem() {
    for (auto& vb : visualBodies) {
        // Безопасное удаление через Qt Event Loop
        if (vb.entity) {
            vb.entity->setParent((Qt3DCore::QEntity*)nullptr);
            vb.entity->deleteLater();
            vb.entity = nullptr;
        }
        if (vb.trail) {
            vb.trail->setParent((Qt3DCore::QEntity*)nullptr);
            vb.trail->deleteLater();
            vb.trail = nullptr;
        }
        if (vb.label) {
            vb.label->setParent((Qt3DCore::QEntity*)nullptr);
            vb.label->deleteLater();
            vb.label = nullptr;
        }
    }
    
    visualBodies.clear();
    physics.bodies.clear();
    selectedBodyIndex = -1;
    updateInfoPanel();
}

void MainWindow::zoomIn() { 
    view3D->camera()->translate(QVector3D(0, 0, 50.0f), Qt3DRender::QCamera::DontTranslateViewCenter); 
}
void MainWindow::zoomOut() { 
    view3D->camera()->translate(QVector3D(0, 0, -50.0f), Qt3DRender::QCamera::DontTranslateViewCenter); 
}
void MainWindow::resetCamera() {
    view3D->camera()->setPosition(QVector3D(0, 400.0f, 400.0f));
    view3D->camera()->setViewCenter(QVector3D(0, 0, 0));
    view3D->camera()->setUpVector(QVector3D(0, 1, 0));
}

void MainWindow::onShowLabelsToggled(bool checked) {
    for (auto& vb : visualBodies) {
        if (vb.label) vb.label->setEnabled(checked);
    }
}

void MainWindow::onShowTrailsToggled(bool checked) {
    for (auto& vb : visualBodies) {
        if (vb.trail) vb.trail->setEnabled(checked);
    }
}

void MainWindow::setupSystem() {
    clearSystem();
    physics.addBody(CelestialBody("Sun", 1.989e30, 696340000, Qt::yellow, {0, 0, 0}, {0, 0, 0}));
    physics.addBody(CelestialBody("Mercury", 3.301e23, 2439700, Qt::lightGray, {5.79e10, 0, 0}, {0, 47400, 0}));
    physics.addBody(CelestialBody("Venus", 4.867e24, 6051800, QColor("#e3bb76"), {1.082e11, 0, 0}, {0, 35020, 0}));
    physics.addBody(CelestialBody("Earth", 5.972e24, 6371000, Qt::blue, {1.496e11, 0, 0}, {0, 29780, 0}));
    physics.addBody(CelestialBody("Mars", 6.417e23, 3389500, Qt::red, {2.279e11, 0, 0}, {0, 24070, 0}));
    physics.addBody(CelestialBody("Ceres", 9.39e20, 473000, Qt::gray, {4.14e11, 0, 0}, {0, 17900, 0}));
    physics.addBody(CelestialBody("Vesta", 2.59e20, 262700, Qt::darkGray, {3.53e11, 0, 0}, {0, 19300, 0}));
    physics.addBody(CelestialBody("Jupiter", 1.898e27, 69911000, QColor("#d8ca9d"), {7.786e11, 0, 0}, {0, 13070, 0}));
    physics.addBody(CelestialBody("Saturn", 5.683e26, 58232000, QColor("#ead6b8"), {1.433e12, 0, 0}, {0, 9690, 0}));
    physics.addBody(CelestialBody("Uranus", 8.681e25, 25362000, QColor("#d1e7e7"), {2.872e12, 0, 0}, {0, 6800, 0}));
    physics.addBody(CelestialBody("Neptune", 1.024e26, 24622000, QColor("#5b5ddf"), {4.495e12, 0, 0}, {0, 5430, 0}));
    physics.addBody(CelestialBody("Pluto", 1.309e22, 1188300, QColor("#968570"), {4.437e12, 0, 1.3e12}, {0, 6100, 0}));
    physics.addBody(CelestialBody("Halley's Comet", 2.2e14, 5500, Qt::white, {8.78e10, 0, 0}, {0, 54500, 0}));
    createVisuals();
}

void MainWindow::toggleSimulation() {
    if (timer->isActive()) { timer->stop(); btnPlayPause->setText("Resume"); }
    else { timer->start(); btnPlayPause->setText("Pause"); }
}
void MainWindow::resetSimulation() { setupSystem(); if (!timer->isActive()) toggleSimulation(); }
void MainWindow::onSpeedChanged(int val) {
    currentSpeedMultiplier = val / 100.0;
    labelSpeed->setText(QString::number(currentSpeedMultiplier, 'f', 1) + "x");
}
void MainWindow::onIntegratorChanged(int index) { physics.currentIntegrator = (index == 0) ? IntegratorType::Verlet : IntegratorType::RungeKutta4; }
void MainWindow::onRelativityToggled(bool checked) { physics.useRelativity = checked; }

void MainWindow::saveSimulation() {
    bool wasRunning = timer->isActive(); if (wasRunning) timer->stop(); 
    QString fileName = QFileDialog::getSaveFileName(this, "Save", "", "JSON (*.json)");
    if (!fileName.isEmpty()) {
        QJsonArray arr;
        for (const auto& b : physics.bodies) {
            QJsonObject o; o["name"] = b.name; o["mass"] = b.mass; o["radius"] = b.radius; o["color"] = b.color.name();
            o["posX"] = b.position.x(); o["posY"] = b.position.y(); o["posZ"] = b.position.z();
            o["velX"] = b.velocity.x(); o["velY"] = b.velocity.y(); o["velZ"] = b.velocity.z();
            arr.append(o);
        }
        QJsonDocument doc(QJsonObject{{"bodies", arr}});
        QFile file(fileName); if (file.open(QIODevice::WriteOnly)) file.write(doc.toJson());
    }
    if (wasRunning) timer->start();
}

void MainWindow::loadSimulation() {
    bool wasRunning = timer->isActive(); if (wasRunning) timer->stop();
    QString fileName = QFileDialog::getOpenFileName(this, "Load", "", "JSON (*.json)");
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly)) {
            QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
            QJsonArray arr = doc.object()["bodies"].toArray();
            clearSystem();
            for (auto v : arr) {
                QJsonObject o = v.toObject();
                Eigen::Vector3d p(o["posX"].toDouble(), o["posY"].toDouble(), o["posZ"].toDouble());
                Eigen::Vector3d v3(o["velX"].toDouble(), o["velY"].toDouble(), o["velZ"].toDouble());
                physics.addBody(CelestialBody(o["name"].toString(), o["mass"].toDouble(), o["radius"].toDouble(), QColor(o["color"].toString()), p, v3));
            }
            createVisuals(); 
        }
    }
    if (wasRunning) timer->start();
}