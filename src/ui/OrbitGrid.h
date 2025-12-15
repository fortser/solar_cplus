#pragma once

#include <Qt3DCore/QEntity>
#include <Qt3DCore/QGeometry>
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DExtras/QPhongMaterial>
#include <QVector3D>
#include <cmath>
#include <vector>

class OrbitGrid : public Qt3DCore::QEntity {
public:
    OrbitGrid(Qt3DCore::QEntity* parent, double scaleFactor) : Qt3DCore::QEntity(parent) {
        
        // Рендерер: рисуем линии
        auto renderer = new Qt3DRender::QGeometryRenderer(this);
        renderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::Lines);

        // Геометрия
        auto geometry = new Qt3DCore::QGeometry(this);
        auto buffer = new Qt3DCore::QBuffer(geometry);
        
        // Генерируем точки для колец
        QByteArray data;
        std::vector<float> points;

        // Список радиусов в АЕ
        std::vector<double> radiiAU = {1.0, 5.2, 9.5, 19.2, 30.0, 40.0}; 

        const int segments = 120; // Количество отрезков в круге

        for (double rAU : radiiAU) {
            double r = rAU * 1.496e11 * scaleFactor; 

            for (int i = 0; i < segments; ++i) {
                // --- ПУНКТИР: Пропускаем каждый второй сегмент ---
                if (i % 2 != 0) continue; 

                double theta1 = 2.0 * M_PI * i / segments;
                double theta2 = 2.0 * M_PI * (i + 1) / segments;

                // Точка начала отрезка
                points.push_back(r * cos(theta1)); 
                points.push_back(0.0f);            
                points.push_back(r * sin(theta1)); 

                // Точка конца отрезка
                points.push_back(r * cos(theta2));
                points.push_back(0.0f);
                points.push_back(r * sin(theta2));
            }
        }

        // Оси координат (сплошные линии)
        double axisLen = 45.0 * 1.496e11 * scaleFactor;
        // Ось X
        points.push_back(-axisLen); points.push_back(0); points.push_back(0);
        points.push_back(axisLen);  points.push_back(0); points.push_back(0);
        // Ось Z
        points.push_back(0); points.push_back(0); points.push_back(-axisLen);
        points.push_back(0); points.push_back(0); points.push_back(axisLen);

        // Загружаем в буфер
        data.resize(points.size() * sizeof(float));
        memcpy(data.data(), points.data(), data.size());
        buffer->setData(data);

        // Атрибут позиции
        auto posAttr = new Qt3DCore::QAttribute(geometry);
        posAttr->setName(Qt3DCore::QAttribute::defaultPositionAttributeName());
        posAttr->setVertexBaseType(Qt3DCore::QAttribute::Float);
        posAttr->setVertexSize(3);
        posAttr->setAttributeType(Qt3DCore::QAttribute::VertexAttribute);
        posAttr->setBuffer(buffer);
        posAttr->setByteStride(3 * sizeof(float));
        posAttr->setCount(points.size() / 3);

        geometry->addAttribute(posAttr);
        renderer->setGeometry(geometry);
        renderer->setVertexCount(points.size() / 3);

        // Материал (ТУСКЛЫЙ ЦВЕТ)
        auto material = new Qt3DExtras::QPhongMaterial(this);
        // Цвет темно-серый (50, 50, 50), чтобы не отвлекать
        QColor gridColor(50, 50, 50); 
        material->setAmbient(gridColor);
        material->setDiffuse(gridColor);
        material->setShininess(0);

        addComponent(renderer);
        addComponent(material);
    }
};