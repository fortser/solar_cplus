#include "MainWindow.h"
#include <QVBoxLayout>
#include <QGraphicsEllipseItem>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    // --- 1. Настройка Layout и Виджетов ---
    auto centralWidget = new QWidget(this);
    auto mainLayout = new QVBoxLayout(centralWidget);
    
    // Графическая сцена
    scene = new QGraphicsScene(this);
    scene->setBackgroundBrush(Qt::black);
    // Устанавливаем границы сцены, чтобы (0,0) было по центру
    scene->setSceneRect(-400, -300, 800, 600);

    view = new QGraphicsView(scene);
    view->setRenderHint(QPainter::Antialiasing);
    view->setDragMode(QGraphicsView::ScrollHandDrag); // Панорамирование
    
    // Добавляем View в лайаут (растягиваем на всё доступное место)
    mainLayout->addWidget(view, 1); 

    // --- Панель управления (Нижняя часть) ---
    auto controlsLayout = new QHBoxLayout();

    // Кнопка Старт/Пауза
    btnPlayPause = new QPushButton("Pause", this);
    connect(btnPlayPause, &QPushButton::clicked, this, &MainWindow::toggleSimulation);
    controlsLayout->addWidget(btnPlayPause);

    // Кнопка Сброс
    btnReset = new QPushButton("Reset", this);
    connect(btnReset, &QPushButton::clicked, this, &MainWindow::resetSimulation);
    controlsLayout->addWidget(btnReset);

    // Слайдер скорости
    controlsLayout->addWidget(new QLabel("Speed:", this));
    sliderSpeed = new QSlider(Qt::Horizontal, this);
    sliderSpeed->setRange(0, 500); // От 0% до 500% скорости
    sliderSpeed->setValue(100);    // По умолчанию 100%
    connect(sliderSpeed, &QSlider::valueChanged, this, &MainWindow::onSpeedChanged);
    controlsLayout->addWidget(sliderSpeed);

    // Метка текущей скорости
    labelSpeed = new QLabel("1.0x (1 day/tick)", this);
    labelSpeed->setMinimumWidth(100);
    controlsLayout->addWidget(labelSpeed);

    mainLayout->addLayout(controlsLayout); // Добавляем панель вниз
    setCentralWidget(centralWidget);
    
    resize(1024, 768);
    setWindowTitle("Solar Orbital Simulator (MVP) - Controls Added");

    // --- 2. Логика ---
    setupSystem();

    // Таймер (~60 FPS)
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateSimulation);
    timer->start(16); 
}

void MainWindow::clearSystem() {
    // Удаляем графические элементы со сцены
    for (auto item : bodyItems) {
        scene->removeItem(item);
        delete item;
    }
    bodyItems.clear();
    
    // Очищаем физический движок
    physics.bodies.clear();
}

void MainWindow::setupSystem() {
    clearSystem(); // На случай рестарта

    // Данные: Солнце
    double mSun = 1.989e30;
    // Данные: Земля
    double mEarth = 5.972e24;
    double distEarth = 1.496e11; 
    double vEarth = 29780.0;     

    // Добавляем Солнце (желтое)
    physics.addBody(CelestialBody("Sun", mSun, 696340000, Qt::yellow, {0, 0}, {0, 0}));

    // Добавляем Землю (синяя)
    physics.addBody(CelestialBody("Earth", mEarth, 6371000, Qt::blue, {distEarth, 0}, {0, vEarth}));

    // Добавляем Марс (красный) - БОНУС для наглядности
    double mMars = 6.39e23;
    double distMars = 2.279e11; // ~1.52 AE
    double vMars = 24077.0;     // ~24 км/с
    physics.addBody(CelestialBody("Mars", mMars, 3389500, Qt::red, {distMars, 0}, {0, vMars}));

    // Создаем графику
    for (const auto& body : physics.bodies) {
        double visualSize = (body.name == "Sun") ? 30.0 : 10.0;
        if (body.name == "Mars") visualSize = 8.0;

        QGraphicsEllipseItem* item = scene->addEllipse(
            -visualSize/2, -visualSize/2, visualSize, visualSize, 
            Qt::NoPen, QBrush(body.color)
        );
        // Добавляем тултип (всплывающую подсказку при наведении)
        item->setToolTip(body.name); 
        bodyItems.push_back(item);
    }
    
    drawBodies();
}

void MainWindow::updateSimulation() {
    // Вычисляем dt на основе слайдера
    // slider=100 -> multiplier=1.0 -> dt = 1 день
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
    setupSystem(); // Сбрасываем позиции в начальные
    if (!timer->isActive()) {
        toggleSimulation(); // Автостарт при сбросе
    }
}

void MainWindow::onSpeedChanged(int val) {
    currentSpeedMultiplier = val / 100.0;
    
    // Обновляем текст метки
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