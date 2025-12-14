#pragma once
#include <QGraphicsView>
#include <QWheelEvent>

class InteractiveView : public QGraphicsView {
public:
    InteractiveView(QGraphicsScene* scene, QWidget* parent = nullptr) 
        : QGraphicsView(scene, parent) {
        
        // Включаем сглаживание для красивых линий
        setRenderHint(QPainter::Antialiasing);
        
        // Разрешаем перетаскивание сцены мышкой (панорамирование)
        setDragMode(QGraphicsView::ScrollHandDrag);
        
        // Убираем скроллбары, чтобы они не мешали (опционально)
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        // Важная настройка: Зум происходит относительно позиции курсора мыши
        setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    }

protected:
    // Переопределяем событие колесика мыши
    void wheelEvent(QWheelEvent* event) override {
        // Коэффициент масштабирования (1.2 означает увеличение на 20%)
        const double scaleFactor = 1.15;

        if (event->angleDelta().y() > 0) {
            // Колесико от себя -> Приближаем
            scale(scaleFactor, scaleFactor);
        } else {
            // Колесико на себя -> Отдаляем
            scale(1.0 / scaleFactor, 1.0 / scaleFactor);
        }
        
        // Не вызываем базовый класс, чтобы не сработала прокрутка скроллбаров
    }
};