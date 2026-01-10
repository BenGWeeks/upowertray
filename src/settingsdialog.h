#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QComboBox>

struct SystemPowerSettings {
    // UPower settings
    double percentageLow = 20.0;
    double percentageCritical = 5.0;
    double percentageAction = 2.0;
    QString criticalAction = "HybridSleep";
    // Logind settings
    QString handleLidSwitch = "suspend";
    QString handleLidSwitchExternalPower = "suspend";
    // Power profile
    QString powerProfile = "Unknown";
};

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(int batteryPercent, bool charging, QWidget *parent = nullptr);

private slots:
    void onPowerProfileChanged(int index);

private:
    QIcon createBatteryIcon(int percentage, bool charging, int lowThreshold, int criticalThreshold);
    SystemPowerSettings readSystemSettings();
    QStringList getAvailablePowerProfiles();
    void setPowerProfile(const QString &profile);

    QLabel *batteryIconLabel;
    QLabel *batteryTextLabel;
    QComboBox *profileCombo;
};

#endif // SETTINGSDIALOG_H
