#ifndef BATTERYTRAY_H
#define BATTERYTRAY_H

#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QTimer>

class BatteryTray : public QObject
{
    Q_OBJECT

public:
    explicit BatteryTray(QObject *parent = nullptr);
    ~BatteryTray();

private slots:
    void updateBattery();
    void onActivated(QSystemTrayIcon::ActivationReason reason);
    void showSettings();
    void quit();

private:
    void createTrayIcon();
    void createMenu();
    void showLowBatteryNotification(int percentage);
    void loadSystemSettings();
    void findBatteryDevice();

    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;
    QTimer *updateTimer;

    QString batteryDevicePath;

    int lastPercentage;
    bool lastCharging;
    bool lowBatteryWarningShown;
    bool criticalBatteryWarningShown;

    // Read from UPower config
    int lowBatteryThreshold;
    int criticalBatteryThreshold;
    static constexpr int UPDATE_INTERVAL_MS = 30000; // 30 seconds
};

#endif // BATTERYTRAY_H
