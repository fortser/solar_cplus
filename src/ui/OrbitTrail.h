#pragma once

#include <Qt3DCore/QEntity>
#include <Qt3DCore/QGeometry>
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DExtras/QPhongMaterial>
#include <QVector3D>
#include <QByteArray>

class OrbitTrail : public Qt3DCore::QEntity {
public:
    OrbitTrail(Qt3DCore::QEntity* parent, QColor color, int maxPoints = 5000)
        : Qt3DCore::QEntity(parent), m_maxPoints(maxPoints) {
        
        // 1. Рендерер (Рисовальщик)
        m_renderer = new Qt3DRender::QGeometryRenderer(this);
        m_renderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::LineStrip); // Линия точка-за-точкой

        // 2. Геометрия (Данные)
        m_geometry = new Qt3DCore::QGeometry(this);
        m_buffer = new Qt3DCore::QBuffer(m_geometry);
        
        // Атрибут позиции (x, y, z)
        m_posAttribute = new Qt3DCore::QAttribute(m_geometry);
        m_posAttribute->setName(Qt3DCore::QAttribute::defaultPositionAttributeName());
        m_posAttribute->setVertexBaseType(Qt3DCore::QAttribute::Float);
        m_posAttribute->setVertexSize(3); // x, y, z
        m_posAttribute->setAttributeType(Qt3DCore::QAttribute::VertexAttribute);
        m_posAttribute->setBuffer(m_buffer);
        
        // Связываем
        m_geometry->addAttribute(m_posAttribute);
        m_renderer->setGeometry(m_geometry);

        // 3. Материал (Цвет)
        m_material = new Qt3DExtras::QPhongMaterial(this);
        m_material->setAmbient(color); // Цвет линии
        m_material->setDiffuse(color);
        m_material->setShininess(0);   // Линия не блестит

        // Собираем компоненты
        addComponent(m_renderer);
        addComponent(m_material);
    }

    // Метод для добавления новой точки
    void update(QVector3D newPos) {
        // Добавляем точку в историю
        m_points.push_back(newPos);
        
        // Если точек слишком много, удаляем старые (чтобы хвост не был бесконечным)
        if (m_points.size() > m_maxPoints) {
            m_points.erase(m_points.begin()); // Удаляем самую старую
        }

        // Обновляем буфер GPU (это самая затратная операция, но для линий ок)
        updateBuffer();
    }

    // Очистка траектории (при ресете)
    void clear() {
        m_points.clear();
        updateBuffer();
    }

private:
    int m_maxPoints;
    std::vector<QVector3D> m_points;

    Qt3DRender::QGeometryRenderer* m_renderer;
    Qt3DCore::QGeometry* m_geometry;
    Qt3DCore::QBuffer* m_buffer;
    Qt3DCore::QAttribute* m_posAttribute;
    Qt3DExtras::QPhongMaterial* m_material;

    void updateBuffer() {
        // Конвертируем vector<QVector3D> в сырые байты (float array)
        QByteArray data;
        data.resize(m_points.size() * 3 * sizeof(float));
        
        float* rawData = reinterpret_cast<float*>(data.data());
        for (const auto& p : m_points) {
            *rawData++ = p.x();
            *rawData++ = p.y();
            *rawData++ = p.z();
        }

        // Загружаем в видеопамять
        m_buffer->setData(data);
        
        // Сообщаем рендереру, сколько точек рисовать
        m_renderer->setVertexCount(m_points.size());
        
        // Шаг между точками в байтах (3 float * 4 byte = 12)
        m_posAttribute->setByteStride(3 * sizeof(float)); 
    }
};