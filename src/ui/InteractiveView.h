#pragma once
#include <QGraphicsView>
#include <QWheelEvent>

class InteractiveView : public QGraphicsView {
    Q_OBJECT // Нужен для слотов
public:
    InteractiveView(QGraphicsScene* scene, QWidget* parent = nullptr) 
        : QGraphicsView(scene, parent) {
        
        setRenderHint(QPainter::Antialiasing);
        setDragMode(QGraphicsView::ScrollHandDrag);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    }

public slots:
    // Публичные методы для кнопок
    void zoomIn() {
        scale(1.15, 1.15);
    }

    void zoomOut() {
        scale(1.0 / 1.15, 1.0 / 1.15);
    }

protected:
    void wheelEvent(QWheelEvent* event) override {
        // Если крутим колесико
        if (event->angleDelta().y() > 0) {
            zoomIn();
        } else {
            zoomOut();
        }
    }
};