#ifndef BATTERYTRAY_H
#define BATTERYTRAY_H

#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QTimer>
#include <QDBusInterface>
#include <QDBusConnection>

class BatteryTray : public QObject
{
    Q_OBJECT

public:
    explicit BatteryTray(QObject *parent = nullptr);
    ~BatteryTray();

private slots:
    void updateBattery();
    void onActivated(QSystemTrayIcon::ActivationReason reason);
    void quit();

private:
    void createTrayIcon();
    void createMenu();
    QIcon createBatteryIcon(int percentage, bool charging);
    void showLowBatteryNotification(int percentage);

    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;
    QTimer *updateTimer;
    QDBusInterface *upowerDevice;

    int lastPercentage;
    bool lowBatteryWarningShown;
    bool criticalBatteryWarningShown;

    static constexpr int LOW_BATTERY_THRESHOLD = 20;
    static constexpr int CRITICAL_BATTERY_THRESHOLD = 5;
    static constexpr int UPDATE_INTERVAL_MS = 30000; // 30 seconds
};

#endif // BATTERYTRAY_H
