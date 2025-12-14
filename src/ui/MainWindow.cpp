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
    // Делаем сцену огромной, чтобы хватило места до Нептуна и комет
    scene->setSceneRect(-500000, -500000, 1000000, 1000000);

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

    // Группа 2: Файлы
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
    setWindowTitle("Solar Orbital Simulator (MVP) - Full Solar System");

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

// Создание визуальных объектов (шариков)
void MainWindow::createVisuals() {
    for (const auto& body : physics.bodies) {
        double visualSize = 6.0; // Размер по умолчанию (астероиды)

        // Настройка размера точек (пиксели)
        if (body.name == "Sun") visualSize = 40.0;
        else if (body.name == "Jupiter") visualSize = 18.0;
        else if (body.name == "Saturn") visualSize = 16.0;
        else if (body.name == "Uranus" || body.name == "Neptune") visualSize = 12.0;
        else if (body.name == "Earth" || body.name == "Venus") visualSize = 9.0;
        else if (body.name == "Mercury" || body.name == "Mars") visualSize = 7.0;
        else if (body.name == "Halley's Comet") visualSize = 5.0;

        QGraphicsEllipseItem* item = scene->addEllipse(
            -visualSize/2, -visualSize/2, visualSize, visualSize, 
            Qt::NoPen, QBrush(body.color)
        );
        
        // Всплывающая подсказка при наведении мыши
        item->setToolTip(body.name);
        
        // ВАЖНО: Эта настройка делает так, что при зуме размер точки в пикселях не меняется.
        // Это позволяет видеть Плутон, даже если зум очень маленький (вид всей системы).
        item->setFlag(QGraphicsItem::ItemIgnoresTransformations, true); 
        
        bodyItems.push_back(item);
    }
    drawBodies(); // Сразу расставляем по местам
}

// Инициализация Солнечной системы
void MainWindow::setupSystem() {
    clearSystem();

    // --- 1. Солнце ---
    double mSun = 1.989e30;
    physics.addBody(CelestialBody("Sun", mSun, 696340000, Qt::yellow, {0, 0}, {0, 0}));

    // --- 2. Планеты земной группы ---
    // Меркурий
    physics.addBody(CelestialBody("Mercury", 3.301e23, 2439700, Qt::lightGray, 
                                  {5.79e10, 0}, {0, 47400}));
    // Венера
    physics.addBody(CelestialBody("Venus", 4.867e24, 6051800, QColor("#e3bb76"), 
                                  {1.082e11, 0}, {0, 35020}));
    // Земля
    physics.addBody(CelestialBody("Earth", 5.972e24, 6371000, Qt::blue, 
                                  {1.496e11, 0}, {0, 29780}));
    // Марс
    physics.addBody(CelestialBody("Mars", 6.417e23, 3389500, Qt::red, 
                                  {2.279e11, 0}, {0, 24070}));

    // --- 3. Астероиды (Главный пояс) ---
    // Церера
    physics.addBody(CelestialBody("Ceres", 9.39e20, 473000, Qt::gray, 
                                  {4.14e11, 0}, {0, 17900}));
    // Веста
    physics.addBody(CelestialBody("Vesta", 2.59e20, 262700, Qt::darkGray, 
                                  {3.53e11, 0}, {0, 19300}));
    // Паллада
    physics.addBody(CelestialBody("Pallas", 2.11e20, 256000, Qt::darkGray, 
                                  {4.14e11, 0}, {0, 17600}));

    // --- 4. Планеты-гиганты ---
    // Юпитер
    physics.addBody(CelestialBody("Jupiter", 1.898e27, 69911000, QColor("#d8ca9d"), 
                                  {7.786e11, 0}, {0, 13070}));
    // Сатурн
    physics.addBody(CelestialBody("Saturn", 5.683e26, 58232000, QColor("#ead6b8"), 
                                  {1.433e12, 0}, {0, 9690}));
    // Уран
    physics.addBody(CelestialBody("Uranus", 8.681e25, 25362000, QColor("#d1e7e7"), 
                                  {2.872e12, 0}, {0, 6800}));
    // Нептун
    physics.addBody(CelestialBody("Neptune", 1.024e26, 24622000, QColor("#5b5ddf"), 
                                  {4.495e12, 0}, {0, 5430}));

    // --- 5. Транснептуновые объекты и Кометы ---
    // Плутон (Карликовая планета)
    physics.addBody(CelestialBody("Pluto", 1.309e22, 1188300, QColor("#968570"), 
                                  {4.437e12, 0}, {0, 6100}));

    // Комета Галлея (Сильно вытянутая орбита)
    // Перигелий (~0.58 АЕ от Солнца)
    physics.addBody(CelestialBody("Halley's Comet", 2.2e14, 5500, Qt::white, 
                                  {8.78e10, 0}, {0, 54500}));

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
    if (wasRunning) timer->stop(); 

    QString fileName = QFileDialog::getSaveFileName(this, "Save Simulation", "", "JSON Files (*.json)");
    
    if (!fileName.isEmpty()) {
        QJsonArray bodiesArray;
        
        for (const auto& body : physics.bodies) {
            QJsonObject obj;
            obj["name"] = body.name;
            obj["mass"] = body.mass;
            obj["radius"] = body.radius;
            obj["color"] = body.color.name(); 
            
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
                clearSystem(); 
                
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
                
                createVisuals(); 
                view->centerOn(0,0);
            }
        }
    }

    if (wasRunning) timer->start();
}