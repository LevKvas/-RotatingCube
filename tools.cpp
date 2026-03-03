#include <QPixmap>
#include <QImage>
#include <QColor>

QPixmap makeTransparent(const QPixmap& source, const QColor& bgColor = Qt::white, int tolerance = 0) {
    if (source.isNull()) return source;

    QImage image = source.toImage().convertToFormat(QImage::Format_ARGB32);

    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            QColor pixelColor(image.pixel(x, y));

            if (tolerance == 0) {
                if (pixelColor == bgColor) {
                    image.setPixel(x, y, qRgba(0, 0, 0, 0));
                }
            } else {
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
