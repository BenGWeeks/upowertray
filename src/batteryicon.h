#ifndef BATTERYICON_H
#define BATTERYICON_H

#include <QIcon>
#include <QColor>

/**
 * Shared battery icon rendering utility.
 * Eliminates duplicate icon rendering code between BatteryTray and SettingsDialog.
 */
class BatteryIcon
{
public:
    struct Thresholds {
        int low = 20;
        int critical = 5;
    };

    /**
     * Create a battery icon with the given parameters.
     * @param size Icon size in pixels (square)
     * @param percentage Battery percentage (0-100)
     * @param charging Whether the battery is charging
     * @param thresholds Low and critical battery thresholds
     * @return QIcon with the rendered battery
     */
    static QIcon create(int size, int percentage, bool charging, const Thresholds &thresholds);

    /**
     * Get the appropriate color for the given battery percentage.
     * @param percentage Battery percentage (0-100)
     * @param thresholds Low and critical battery thresholds
     * @return Color (green, amber, or red)
     */
    static QColor getColor(int percentage, const Thresholds &thresholds);

    /**
     * Get the outline color (white when healthy, colored when low/critical).
     * @param percentage Battery percentage (0-100)
     * @param thresholds Low and critical battery thresholds
     * @return Color for the outline
     */
    static QColor getOutlineColor(int percentage, const Thresholds &thresholds);

private:
    static const QColor COLOR_GREEN;
    static const QColor COLOR_AMBER;
    static const QColor COLOR_RED;
};

#endif // BATTERYICON_H
