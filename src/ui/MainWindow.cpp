#include "MainWindow.h"
#include <QVBoxLayout>
#include <QGraphicsEllipseItem>
#include <QFile>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    // --- 1. Настройка Layout ---
    auto centralWidget = new QWidget(this);
    auto mainLayout = new QVBoxLayout(centralWidget);
    
    scene = new QGraphicsScene(this);
    scene->setBackgroundBrush(Qt::black);
    scene->setSceneRect(-50000, -50000, 100000, 100000);

    view = new InteractiveView(scene, this);
    mainLayout->addWidget(view, 1); 

    // --- Панель управления ---
    auto controlsLayout = new QHBoxLayout();

    // Группа 1: Управление симуляцией
    btnPlayPause = new QPushButton("Pause", this);
    connect(btnPlayPause, &QPushButton::clicked, this, &MainWindow::toggleSimulation);
    controlsLayout->addWidget(btnPlayPause);

    btnReset = new QPushButton("Reset", this);
    connect(btnReset, &QPushButton::clicked, this, &MainWindow::resetSimulation);
    controlsLayout->addWidget(btnReset);

    // Группа 2: Файлы (Новые кнопки)
    btnSave = new QPushButton("Save...", this);
    connect(btnSave, &QPushButton::clicked, this, &MainWindow::saveSimulation);
    controlsLayout->addWidget(btnSave);

    btnLoad = new QPushButton("Load...", this);
    connect(btnLoad, &QPushButton::clicked, this, &MainWindow::loadSimulation);
    controlsLayout->addWidget(btnLoad);

    // Разделитель
    controlsLayout->addSpacing(20);

    // Группа 3: Скорость
    controlsLayout->addWidget(new QLabel("Speed:", this));
    sliderSpeed = new QSlider(Qt::Horizontal, this);
    sliderSpeed->setRange(0, 500); 
    sliderSpeed->setValue(100);    
    connect(sliderSpeed, &QSlider::valueChanged, this, &MainWindow::onSpeedChanged);
    controlsLayout->addWidget(sliderSpeed);

    labelSpeed = new QLabel("1.0x (1 day/tick)", this);
    labelSpeed->setMinimumWidth(120);
    controlsLayout->addWidget(labelSpeed);

    mainLayout->addLayout(controlsLayout); 
    setCentralWidget(centralWidget);
    
    resize(1024, 768);
    setWindowTitle("Solar Orbital Simulator (MVP) - Save/Load System");

    // --- 2. Логика ---
    setupSystem();
    view->centerOn(0, 0);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateSimulation);
    timer->start(16); 
}

void MainWindow::clearSystem() {
    for (auto item : bodyItems) {
        scene->removeItem(item);
        delete item;
    }
    bodyItems.clear();
    physics.bodies.clear();
}

// Эта функция создает "шарики" на экране для всех тел, которые есть в физическом движке
void MainWindow::createVisuals() {
    for (const auto& body : physics.bodies) {
        double visualSize = 10.0;
        if (body.name == "Sun") visualSize = 30.0;
        else if (body.name == "Mars") visualSize = 8.0;

        QGraphicsEllipseItem* item = scene->addEllipse(
            -visualSize/2, -visualSize/2, visualSize, visualSize, 
            Qt::NoPen, QBrush(body.color)
        );
        item->setToolTip(body.name);
        item->setFlag(QGraphicsItem::ItemIgnoresTransformations, false); 
        bodyItems.push_back(item);
    }
    drawBodies(); // Сразу ставим их на места
}

void MainWindow::setupSystem() {
    clearSystem();

    // Стандартные данные
    double mSun = 1.989e30;
    double mEarth = 5.972e24;
    double distEarth = 1.496e11; 
    double vEarth = 29780.0;     

    physics.addBody(CelestialBody("Sun", mSun, 696340000, Qt::yellow, {0, 0}, {0, 0}));
    physics.addBody(CelestialBody("Earth", mEarth, 6371000, Qt::blue, {distEarth, 0}, {0, vEarth}));
    
    double mMars = 6.39e23;
    double distMars = 2.279e11; 
    double vMars = 24077.0;     
    physics.addBody(CelestialBody("Mars", mMars, 3389500, Qt::red, {distMars, 0}, {0, vMars}));

    createVisuals();
}

void MainWindow::updateSimulation() {
    double dt = baseTimeStep * currentSpeedMultiplier;
    physics.step(dt);
    drawBodies();
}

void MainWindow::toggleSimulation() {
    if (timer->isActive()) {
        timer->stop();
        btnPlayPause->setText("Resume");
    } else {
        timer->start();
        btnPlayPause->setText("Pause");
    }
}

void MainWindow::resetSimulation() {
    setupSystem();
    view->centerOn(0,0);
    if (!timer->isActive()) toggleSimulation();
}

void MainWindow::onSpeedChanged(int val) {
    currentSpeedMultiplier = val / 100.0;
    QString text = QString::number(currentSpeedMultiplier, 'f', 2) + "x";
    if (val == 0) text += " (Paused)";
    else text += " (" + QString::number(currentSpeedMultiplier, 'f', 1) + " days/tick)";
    labelSpeed->setText(text);
}

void MainWindow::drawBodies() {
    for (size_t i = 0; i < physics.bodies.size(); ++i) {
        double x = physics.bodies[i].position.x() * scaleFactor;
        double y = physics.bodies[i].position.y() * scaleFactor;
        bodyItems[i]->setPos(x, -y);
    }
}

// --- ЛОГИКА СОХРАНЕНИЯ ---
void MainWindow::saveSimulation() {
    bool wasRunning = timer->isActive();
    if (wasRunning) timer->stop(); // Пауза при сохранении

    QString fileName = QFileDialog::getSaveFileName(this, "Save Simulation", "", "JSON Files (*.json)");
    
    if (!fileName.isEmpty()) {
        QJsonArray bodiesArray;
        
        for (const auto& body : physics.bodies) {
            QJsonObject obj;
            obj["name"] = body.name;
            obj["mass"] = body.mass;
            obj["radius"] = body.radius;
            obj["color"] = body.color.name(); // Сохраняем цвет как HEX строку (#FFFF00)
            
            // Сохраняем векторы
            obj["posX"] = body.position.x();
            obj["posY"] = body.position.y();
            obj["velX"] = body.velocity.x();
            obj["velY"] = body.velocity.y();
            
            bodiesArray.append(obj);
        }
        
        QJsonObject root;
        root["bodies"] = bodiesArray;
        
        QJsonDocument doc(root);
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(doc.toJson());
            file.close();
        }
    }

    if (wasRunning) timer->start();
}

// --- ЛОГИКА ЗАГРУЗКИ ---
void MainWindow::loadSimulation() {
    bool wasRunning = timer->isActive();
    if (wasRunning) timer->stop();

    QString fileName = QFileDialog::getOpenFileName(this, "Load Simulation", "", "JSON Files (*.json)");
    
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray data = file.readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);
            QJsonObject root = doc.object();
            
            if (root.contains("bodies") && root["bodies"].isArray()) {
                clearSystem(); // Удаляем текущие планеты
                
                QJsonArray bodiesArray = root["bodies"].toArray();
                for (const auto& val : bodiesArray) {
                    QJsonObject obj = val.toObject();
                    
                    QString name = obj["name"].toString();
                    double mass = obj["mass"].toDouble();
                    double radius = obj["radius"].toDouble();
                    QColor color(obj["color"].toString());
                    
                    Eigen::Vector2d pos(obj["posX"].toDouble(), obj["posY"].toDouble());
                    Eigen::Vector2d vel(obj["velX"].toDouble(), obj["velY"].toDouble());
                    
                    physics.addBody(CelestialBody(name, mass, radius, color, pos, vel));
                }
                
                createVisuals(); // Создаем графику для загруженных планет
                view->centerOn(0,0);
            }
        }
    }

    if (wasRunning) timer->start();
}