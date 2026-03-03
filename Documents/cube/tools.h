#pragma once

#include <QPixmap>
#include <QImage>
#include <QColor>
#include <QtMath>
#include <QVector3D>
#include <QMatrix4x4>


inline QPixmap makeTransparent(const QPixmap& source, const QColor& bgColor = Qt::white, int tolerance = 0) {
    if (source.isNull()) return source;

    QImage image = source.toImage().convertToFormat(QImage::Format_ARGB32);

    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            QColor pixelColor(image.pixel(x, y));

            // Если цвет совпадает с фоном (с учётом tolerance)
            if (tolerance == 0) {
                if (pixelColor == bgColor) {
                    image.setPixel(x, y, qRgba(0, 0, 0, 0));  // Полностью прозрачный
                }
            } else {
                // С допуском: проверяем расстояние в RGB-пространстве
                int dr = pixelColor.red() - bgColor.red();
                int dg = pixelColor.green() - bgColor.green();
                int db = pixelColor.blue() - bgColor.blue();
                int distance = qSqrt(dr*dr + dg*dg + db*db);

                if (distance <= tolerance) {
                    image.setPixel(x, y, qRgba(0, 0, 0, 0));
                }
            }
        }
    }

    return QPixmap::fromImage(image);
}

inline QPoint projectWorldToScreen(const QVector3D& worldPos,
                            const QMatrix4x4& projection,
                            const QMatrix4x4& view,
                            const QMatrix4x4& model,
                            int windowWidth,
                            int windowHeight)
{
    // 1. MVP матрица
    QMatrix4x4 mvp = projection * view * model;

    // 2. Преобразуем точку в clip space
    QVector4D clipPos = mvp * QVector4D(worldPos, 1.0f);

    if (qFuzzyIsNull(clipPos.w())) {
        return QPoint(-1, -1);
    }

    // 3. Perspective divide → NDC [-1, 1]
    QVector3D ndc;
    ndc.setX(clipPos.x() / clipPos.w());
    ndc.setY(clipPos.y() / clipPos.w());
    ndc.setZ(clipPos.z() / clipPos.w());

    // 4. Проверка: точка перед камерой?
    if (ndc.z() < -1.0f || ndc.z() > 1.0f) {
        return QPoint(-1, -1);  // Не видима
    }

    // 5. NDC → экран [0, width/height], инвертируем Y для Qt
    int screenX = static_cast<int>((ndc.x() + 1.0f) * 0.5f * windowWidth);
    int screenY = static_cast<int>((1.0f - ndc.y()) * 0.5f * windowHeight);

    return QPoint(screenX, screenY);
}
