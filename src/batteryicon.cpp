#include "batteryicon.h"
#include <QPainter>
#include <QPixmap>
#include <QPolygon>

const QColor BatteryIcon::COLOR_GREEN(50, 200, 50);
const QColor BatteryIcon::COLOR_AMBER(220, 180, 50);
const QColor BatteryIcon::COLOR_RED(220, 50, 50);

QColor BatteryIcon::getColor(int percentage, const Thresholds &thresholds)
{
    if (percentage <= thresholds.critical) {
        return COLOR_RED;
    } else if (percentage <= thresholds.low) {
        return COLOR_AMBER;
    }
    return COLOR_GREEN;
}

QColor BatteryIcon::getOutlineColor(int percentage, const Thresholds &thresholds)
{
    if (percentage <= thresholds.critical) {
        return COLOR_RED;
    } else if (percentage <= thresholds.low) {
        return COLOR_AMBER;
    }
    return Qt::white;
}

QIcon BatteryIcon::create(int size, int percentage, bool charging, const Thresholds &thresholds)
{
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // Scale factors based on size (reference size is 22px for tray, 64px for dialog)
    const double scale = size / 22.0;

    // Battery body dimensions (scaled)
    const int bodyLeft = static_cast<int>(2 * scale);
    const int bodyTop = static_cast<int>(4 * scale);
    const int bodyWidth = static_cast<int>(16 * scale);
    const int bodyHeight = static_cast<int>(14 * scale);
    const int tipWidth = static_cast<int>(2 * scale);
    const int tipHeight = static_cast<int>(6 * scale);
    const int cornerRadius = static_cast<int>(2 * scale);
    const double penWidth = 1.5 * scale;

    QColor outlineColor = getOutlineColor(percentage, thresholds);
    QColor fillColor = getColor(percentage, thresholds);

    // Draw battery outline
    painter.setPen(QPen(outlineColor, penWidth));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(bodyLeft, bodyTop, bodyWidth, bodyHeight, cornerRadius, cornerRadius);

    // Draw battery tip (positive terminal)
    painter.fillRect(bodyLeft + bodyWidth, bodyTop + (bodyHeight - tipHeight) / 2,
                     tipWidth, tipHeight, outlineColor);

    // Calculate fill dimensions
    const int fillMargin = static_cast<int>(2 * scale);
    const int maxFillWidth = bodyWidth - 2 * fillMargin;
    const int fillWidth = (percentage * maxFillWidth) / 100;
    const int fillHeight = bodyHeight - 2 * fillMargin;

    // Draw fill
    painter.fillRect(bodyLeft + fillMargin, bodyTop + fillMargin,
                     fillWidth, fillHeight, fillColor);

    // Draw charging indicator (lightning bolt)
    if (charging) {
        QColor boltColor = (percentage > thresholds.low) ? Qt::white : COLOR_GREEN;
        painter.setPen(QPen(boltColor, penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.setBrush(boltColor);

        // Scale bolt coordinates
        QPolygon bolt;
        bolt << QPoint(static_cast<int>(12 * scale), static_cast<int>(6 * scale))
             << QPoint(static_cast<int>(8 * scale), static_cast<int>(11 * scale))
             << QPoint(static_cast<int>(10 * scale), static_cast<int>(11 * scale))
             << QPoint(static_cast<int>(8 * scale), static_cast<int>(16 * scale))
             << QPoint(static_cast<int>(12 * scale), static_cast<int>(11 * scale))
             << QPoint(static_cast<int>(10 * scale), static_cast<int>(11 * scale));
        painter.drawPolygon(bolt);
    }

    painter.end();
    return QIcon(pixmap);
}
