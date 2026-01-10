#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QLabel>

struct SystemPowerSettings {
    // UPower settings
    double percentageLow = 20.0;
    double percentageCritical = 5.0;
    double percentageAction = 2.0;
    QString criticalAction = "HybridSleep";
    // Logind settings
    QString handleLidSwitch = "suspend";
    QString handleLidSwitchExternalPower = "suspend";
};

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(int batteryPercent, bool charging, QWidget *parent = nullptr);

private:
    QIcon createBatteryIcon(int percentage, bool charging, int criticalThreshold);
    SystemPowerSettings readSystemSettings();

    QLabel *batteryIconLabel;
    QLabel *batteryTextLabel;
};

#endif // SETTINGSDIALOG_H
