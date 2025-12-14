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
    scene->setSceneRect(-500000, -500000, 1000000, 1000000);

    // Подключаем сигнал изменения выделения на сцене (для инфо-панели)
    connect(scene, &QGraphicsScene::selectionChanged, this, &MainWindow::onSelectionChanged);

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
    btnSave = new QPushButton("Save", this);
    connect(btnSave, &QPushButton::clicked, this, &MainWindow::saveSimulation);
    controlsLayout->addWidget(btnSave);

    btnLoad = new QPushButton("Load", this);
    connect(btnLoad, &QPushButton::clicked, this, &MainWindow::loadSimulation);
    controlsLayout->addWidget(btnLoad);

    // Разделитель
    controlsLayout->addSpacing(15);

    // Группа 3: Зум (НОВЫЕ КНОПКИ)
    btnZoomIn = new QPushButton("(+)", this);
    btnZoomIn->setFixedWidth(30);
    connect(btnZoomIn, &QPushButton::clicked, view, &InteractiveView::zoomIn);
    controlsLayout->addWidget(btnZoomIn);

    btnZoomOut = new QPushButton("(-)", this);
    btnZoomOut->setFixedWidth(30);
    connect(btnZoomOut, &QPushButton::clicked, view, &InteractiveView::zoomOut);
    controlsLayout->addWidget(btnZoomOut);

    // Разделитель
    controlsLayout->addSpacing(15);

    // Группа 4: Скорость
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
    
    // --- Статус бар (Инфо) ---
    labelInfo = new QLabel("Hover over an object to see details", this);
    labelInfo->setStyleSheet("color: gray; font-style: italic;");
    mainLayout->addWidget(labelInfo);

    setCentralWidget(centralWidget);
    
    resize(1024, 768);
    setWindowTitle("Solar Orbital Simulator (MVP)");

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

    // Удаляем текстовые метки
    for (auto label : nameLabels) {
        scene->removeItem(label);
        delete label;
    }
    nameLabels.clear();

    physics.bodies.clear();
}

void MainWindow::createVisuals() {
    // Шрифт для подписей
    QFont font("Arial", 8);
    font.setBold(true);

    for (const auto& body : physics.bodies) {
        double visualSize = 6.0; 
        if (body.name == "Sun") visualSize = 40.0;
        else if (body.name == "Jupiter") visualSize = 18.0;
        else if (body.name == "Saturn") visualSize = 16.0;
        else if (body.name == "Uranus" || body.name == "Neptune") visualSize = 12.0;
        else if (body.name == "Earth" || body.name == "Venus") visualSize = 9.0;
        else if (body.name == "Mercury" || body.name == "Mars") visualSize = 7.0;
        else if (body.name == "Halley's Comet") visualSize = 5.0;

        // 1. Создаем планету
        QGraphicsEllipseItem* item = scene->addEllipse(
            -visualSize/2, -visualSize/2, visualSize, visualSize, 
            Qt::NoPen, QBrush(body.color)
        );
        item->setToolTip(body.name);
        item->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
        
        // Делаем объект выделяемым, чтобы отслеживать клики
        item->setFlag(QGraphicsItem::ItemIsSelectable, true);
        // Записываем имя в данные объекта, чтобы достать при клике
        item->setData(0, body.name); 

        bodyItems.push_back(item);

        // 2. Создаем текстовую подпись
        QGraphicsSimpleTextItem* label = scene->addSimpleText(body.name);
        label->setBrush(Qt::white); // Белый текст
        label->setFont(font);
        // Текст тоже не должен меняться в размере при зуме (всегда читаемый)
        label->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
        
        nameLabels.push_back(label);
    }
    drawBodies(); 
}

void MainWindow::setupSystem() {
    clearSystem();

    // 1. Солнце
    double mSun = 1.989e30;
    physics.addBody(CelestialBody("Sun", mSun, 696340000, Qt::yellow, {0, 0}, {0, 0}));

    // 2. Планеты
    physics.addBody(CelestialBody("Mercury", 3.301e23, 2439700, Qt::lightGray, {5.79e10, 0}, {0, 47400}));
    physics.addBody(CelestialBody("Venus", 4.867e24, 6051800, QColor("#e3bb76"), {1.082e11, 0}, {0, 35020}));
    physics.addBody(CelestialBody("Earth", 5.972e24, 6371000, Qt::blue, {1.496e11, 0}, {0, 29780}));
    physics.addBody(CelestialBody("Mars", 6.417e23, 3389500, Qt::red, {2.279e11, 0}, {0, 24070}));

    // 3. Астероиды
    physics.addBody(CelestialBody("Ceres", 9.39e20, 473000, Qt::gray, {4.14e11, 0}, {0, 17900}));
    physics.addBody(CelestialBody("Vesta", 2.59e20, 262700, Qt::darkGray, {3.53e11, 0}, {0, 19300}));
    physics.addBody(CelestialBody("Pallas", 2.11e20, 256000, Qt::darkGray, {4.14e11, 0}, {0, 17600}));

    // 4. Гиганты
    physics.addBody(CelestialBody("Jupiter", 1.898e27, 69911000, QColor("#d8ca9d"), {7.786e11, 0}, {0, 13070}));
    physics.addBody(CelestialBody("Saturn", 5.683e26, 58232000, QColor("#ead6b8"), {1.433e12, 0}, {0, 9690}));
    physics.addBody(CelestialBody("Uranus", 8.681e25, 25362000, QColor("#d1e7e7"), {2.872e12, 0}, {0, 6800}));
    physics.addBody(CelestialBody("Neptune", 1.024e26, 24622000, QColor("#5b5ddf"), {4.495e12, 0}, {0, 5430}));

    // 5. Прочее
    physics.addBody(CelestialBody("Pluto", 1.309e22, 1188300, QColor("#968570"), {4.437e12, 0}, {0, 6100}));
    physics.addBody(CelestialBody("Halley's Comet", 2.2e14, 5500, Qt::white, {8.78e10, 0}, {0, 54500}));

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
        
        // Обновляем позицию планеты
        bodyItems[i]->setPos(x, -y);

        // Обновляем позицию текста
        // Смещаем текст на (10, -10) пикселей от центра планеты, чтобы не перекрывал её
        nameLabels[i]->setPos(x + 12, -y - 12);
    }
}

// Слот срабатывает, когда мы кликаем по планете (она выделяется)
void MainWindow::onSelectionChanged() {
    QList<QGraphicsItem*> selected = scene->selectedItems();
    if (!selected.isEmpty()) {
        // Берем имя из данных объекта (data(0))
        QString name = selected.first()->data(0).toString();
        labelInfo->setText("Selected: " + name);
        labelInfo->setStyleSheet("color: white; font-weight: bold;");
    } else {
        labelInfo->setText("Hover/Select an object");
        labelInfo->setStyleSheet("color: gray;");
    }
}

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