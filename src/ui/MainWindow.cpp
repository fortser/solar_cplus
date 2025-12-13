#include "MainWindow.h"
#include <QVBoxLayout>
#include <QGraphicsEllipseItem>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    // 1. Настройка UI
    auto centralWidget = new QWidget(this);
    auto layout = new QVBoxLayout(centralWidget);
    
    scene = new QGraphicsScene(this);
    scene->setBackgroundBrush(Qt::black); // Космос черный
    
    view = new QGraphicsView(scene);
    view->setRenderHint(QPainter::Antialiasing);
    // Разрешаем зум и скролл мышкой (базовый функционал View)
    view->setDragMode(QGraphicsView::ScrollHandDrag);

    layout->addWidget(view);
    setCentralWidget(centralWidget);
    resize(800, 600);
    // Устанавливаем сцену так, чтобы (0,0) было в центре
    scene->setSceneRect(-400, -300, 800, 600);
    setWindowTitle("Solar Orbital Simulator (MVP)");

    // 2. Инициализация системы
    setupSystem();

    // 3. Таймер анимации (60 FPS -> ~16 мс)
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateSimulation);
    timer->start(16); 
}

void MainWindow::setupSystem() {
    // Данные: Солнце
    double mSun = 1.989e30;
    // Данные: Земля
    double mEarth = 5.972e24;
    double distEarth = 1.496e11; // 1 АЕ
    double vEarth = 29780.0;     // Орбитальная скорость ~30 км/с

    // Добавляем тела (Позиция X, Y; Скорость VX, VY)
    // Солнце в центре (0,0), скорость 0
    physics.addBody(CelestialBody("Sun", mSun, 696340000, Qt::yellow, 
                                  {0, 0}, {0, 0}));

    // Земля справа (dist, 0), летит вверх (0, v)
    physics.addBody(CelestialBody("Earth", mEarth, 6371000, Qt::blue, 
                                  {distEarth, 0}, {0, vEarth}));

    // Создаем графические элементы для каждого тела
    for (const auto& body : physics.bodies) {
        // Размер точки на экране (не в масштабе, иначе планеты не видно)
        double visualSize = (body.name == "Sun") ? 20.0 : 8.0; 
        
        QGraphicsEllipseItem* item = scene->addEllipse(
            -visualSize/2, -visualSize/2, visualSize, visualSize, 
            Qt::NoPen, QBrush(body.color)
        );
        bodyItems.push_back(item);
    }
}

void MainWindow::updateSimulation() {
    // Шаг времени.
    // В реальности 16 мс экрана != 16 мс симуляции.
    // Пусть 1 шаг таймера = 1 день симуляции (86400 сек) для наглядности
    double dt = 3600 * 24; 

    // Считаем физику
    physics.step(dt);

    // Обновляем графику
    drawBodies();
}

void MainWindow::drawBodies() {
    for (size_t i = 0; i < physics.bodies.size(); ++i) {
        // Переводим метры в пиксели
        double x = physics.bodies[i].position.x() * scaleFactor;
        double y = physics.bodies[i].position.y() * scaleFactor;

        // В Qt ось Y направлена вниз, в физике вверх. Инвертируем Y.
        bodyItems[i]->setPos(x, -y);
    }
    
    // Центрируем камеру на Солнце (опционально)
    // view->centerOn(bodyItems[0]); 
}