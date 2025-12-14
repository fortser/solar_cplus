#include "MainWindow.h"
#include <QVBoxLayout>
#include <QGraphicsEllipseItem>
#include <QFile>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    // --- 1. Настройка Layout и UI ---
    auto centralWidget = new QWidget(this);
    auto mainLayout = new QVBoxLayout(centralWidget);
    
    scene = new QGraphicsScene(this);
    scene->setBackgroundBrush(Qt::black);
    scene->setSceneRect(-500000, -500000, 1000000, 1000000);
    connect(scene, &QGraphicsScene::selectionChanged, this, &MainWindow::onSelectionChanged);

    view = new InteractiveView(scene, this);
    mainLayout->addWidget(view, 1); 

    // Панель управления (Нижняя)
    auto controlsLayout = new QHBoxLayout();

    // Кнопки симуляции
    btnPlayPause = new QPushButton("Pause", this);
    connect(btnPlayPause, &QPushButton::clicked, this, &MainWindow::toggleSimulation);
    controlsLayout->addWidget(btnPlayPause);

    btnReset = new QPushButton("Reset", this);
    connect(btnReset, &QPushButton::clicked, this, &MainWindow::resetSimulation);
    controlsLayout->addWidget(btnReset);

    // Файлы
    btnSave = new QPushButton("Save", this);
    connect(btnSave, &QPushButton::clicked, this, &MainWindow::saveSimulation);
    controlsLayout->addWidget(btnSave);

    btnLoad = new QPushButton("Load", this);
    connect(btnLoad, &QPushButton::clicked, this, &MainWindow::loadSimulation);
    controlsLayout->addWidget(btnLoad);

    controlsLayout->addSpacing(15);

    // Зум
    btnZoomIn = new QPushButton("(+)", this);
    btnZoomIn->setFixedWidth(30);
    connect(btnZoomIn, &QPushButton::clicked, view, &InteractiveView::zoomIn);
    controlsLayout->addWidget(btnZoomIn);

    btnZoomOut = new QPushButton("(-)", this);
    btnZoomOut->setFixedWidth(30);
    connect(btnZoomOut, &QPushButton::clicked, view, &InteractiveView::zoomOut);
    controlsLayout->addWidget(btnZoomOut);

    controlsLayout->addSpacing(15);

    // --- НОВОЕ: Настройки физики ---
    auto physicsLayout = new QVBoxLayout(); 
    
    comboIntegrator = new QComboBox(this);
    comboIntegrator->addItem("Verlet (Fast)");
    comboIntegrator->addItem("Runge-Kutta 4 (Precise)");
    connect(comboIntegrator, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &MainWindow::onIntegratorChanged);
    physicsLayout->addWidget(comboIntegrator);

    checkRelativity = new QCheckBox("Gen. Relativity", this);
    connect(checkRelativity, &QCheckBox::toggled, this, &MainWindow::onRelativityToggled);
    physicsLayout->addWidget(checkRelativity);

    controlsLayout->addLayout(physicsLayout);
    controlsLayout->addSpacing(15);

    // Скорость
    controlsLayout->addWidget(new QLabel("Speed:", this));
    sliderSpeed = new QSlider(Qt::Horizontal, this);
    sliderSpeed->setRange(0, 500); 
    sliderSpeed->setValue(100);    
    connect(sliderSpeed, &QSlider::valueChanged, this, &MainWindow::onSpeedChanged);
    controlsLayout->addWidget(sliderSpeed);

    labelSpeed = new QLabel("1.0x", this);
    labelSpeed->setMinimumWidth(50);
    controlsLayout->addWidget(labelSpeed);

    mainLayout->addLayout(controlsLayout); 
    
    labelInfo = new QLabel("Select an object to see details", this);
    labelInfo->setStyleSheet("color: gray; font-style: italic;");
    mainLayout->addWidget(labelInfo);

    setCentralWidget(centralWidget);
    resize(1200, 800);
    setWindowTitle("Solar Simulator - Step 2.1 (3D Core + RK4)");

    setupSystem();
    view->centerOn(0, 0);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateSimulation);
    timer->start(16); 
}

void MainWindow::clearSystem() {
    for (auto item : bodyItems) { scene->removeItem(item); delete item; }
    bodyItems.clear();
    for (auto label : nameLabels) { scene->removeItem(label); delete label; }
    nameLabels.clear();
    physics.bodies.clear();
}

void MainWindow::createVisuals() {
    QFont font("Arial", 8); font.setBold(true);
    for (const auto& body : physics.bodies) {
        double visualSize = 6.0; 
        if (body.name == "Sun") visualSize = 40.0;
        else if (body.name == "Jupiter") visualSize = 18.0;
        else if (body.name == "Saturn") visualSize = 16.0;
        else if (body.name == "Uranus" || body.name == "Neptune") visualSize = 12.0;
        else if (body.name == "Earth" || body.name == "Venus") visualSize = 9.0;
        else if (body.name == "Mercury" || body.name == "Mars") visualSize = 7.0;
        else if (body.name == "Halley's Comet") visualSize = 5.0;

        QGraphicsEllipseItem* item = scene->addEllipse(-visualSize/2, -visualSize/2, visualSize, visualSize, Qt::NoPen, QBrush(body.color));
        item->setToolTip(body.name);
        item->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
        item->setFlag(QGraphicsItem::ItemIsSelectable, true);
        item->setData(0, body.name); 
        bodyItems.push_back(item);

        QGraphicsSimpleTextItem* label = scene->addSimpleText(body.name);
        label->setBrush(Qt::white); label->setFont(font);
        label->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
        nameLabels.push_back(label);
    }
    drawBodies(); 
}

void MainWindow::setupSystem() {
    clearSystem();
    // Солнце
    physics.addBody(CelestialBody("Sun", 1.989e30, 696340000, Qt::yellow, {0, 0, 0}, {0, 0, 0}));
    // Планеты (3D векторы: x, y, z)
    physics.addBody(CelestialBody("Mercury", 3.301e23, 2439700, Qt::lightGray, {5.79e10, 0, 0}, {0, 47400, 0}));
    physics.addBody(CelestialBody("Venus", 4.867e24, 6051800, QColor("#e3bb76"), {1.082e11, 0, 0}, {0, 35020, 0}));
    physics.addBody(CelestialBody("Earth", 5.972e24, 6371000, Qt::blue, {1.496e11, 0, 0}, {0, 29780, 0}));
    physics.addBody(CelestialBody("Mars", 6.417e23, 3389500, Qt::red, {2.279e11, 0, 0}, {0, 24070, 0}));
    // Астероиды
    physics.addBody(CelestialBody("Ceres", 9.39e20, 473000, Qt::gray, {4.14e11, 0, 0}, {0, 17900, 0}));
    physics.addBody(CelestialBody("Vesta", 2.59e20, 262700, Qt::darkGray, {3.53e11, 0, 0}, {0, 19300, 0}));
    // Гиганты
    physics.addBody(CelestialBody("Jupiter", 1.898e27, 69911000, QColor("#d8ca9d"), {7.786e11, 0, 0}, {0, 13070, 0}));
    physics.addBody(CelestialBody("Saturn", 5.683e26, 58232000, QColor("#ead6b8"), {1.433e12, 0, 0}, {0, 9690, 0}));
    physics.addBody(CelestialBody("Uranus", 8.681e25, 25362000, QColor("#d1e7e7"), {2.872e12, 0, 0}, {0, 6800, 0}));
    physics.addBody(CelestialBody("Neptune", 1.024e26, 24622000, QColor("#5b5ddf"), {4.495e12, 0, 0}, {0, 5430, 0}));
    // Плутон (с наклоном по Z)
    physics.addBody(CelestialBody("Pluto", 1.309e22, 1188300, QColor("#968570"), {4.437e12, 0, 1.3e12}, {0, 6100, 0}));
    // Комета
    physics.addBody(CelestialBody("Halley's Comet", 2.2e14, 5500, Qt::white, {8.78e10, 0, 0}, {0, 54500, 0}));

    createVisuals();
}

void MainWindow::updateSimulation() {
    double dt = baseTimeStep * currentSpeedMultiplier;
    physics.step(dt);
    drawBodies();
}

void MainWindow::drawBodies() {
    for (size_t i = 0; i < physics.bodies.size(); ++i) {
        double x = physics.bodies[i].position.x() * scaleFactor;
        double y = physics.bodies[i].position.y() * scaleFactor;
        bodyItems[i]->setPos(x, -y);
        nameLabels[i]->setPos(x + 12, -y - 12);
    }
}

void MainWindow::toggleSimulation() {
    if (timer->isActive()) { timer->stop(); btnPlayPause->setText("Resume"); }
    else { timer->start(); btnPlayPause->setText("Pause"); }
}

void MainWindow::resetSimulation() { setupSystem(); view->centerOn(0,0); if (!timer->isActive()) toggleSimulation(); }

void MainWindow::onSpeedChanged(int val) {
    currentSpeedMultiplier = val / 100.0;
    labelSpeed->setText(QString::number(currentSpeedMultiplier, 'f', 1) + "x");
}

void MainWindow::onSelectionChanged() {
    QList<QGraphicsItem*> selected = scene->selectedItems();
    if (!selected.isEmpty()) {
        QString name = selected.first()->data(0).toString();
        labelInfo->setText("Selected: " + name);
        labelInfo->setStyleSheet("color: white; font-weight: bold;");
    }
}

void MainWindow::onIntegratorChanged(int index) {
    physics.currentIntegrator = (index == 0) ? IntegratorType::Verlet : IntegratorType::RungeKutta4;
}

void MainWindow::onRelativityToggled(bool checked) {
    physics.useRelativity = checked;
}

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
            createVisuals(); view->centerOn(0,0);
        }
    }
    if (wasRunning) timer->start();
}