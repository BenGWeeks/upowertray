#include "batterytray.h"

#include "batteryicon.h"
#include "settingsdialog.h"
#include "upowerhelper.h"

#include <QApplication>

BatteryTray::BatteryTray(QObject *parent)
    : QObject(parent),
      trayIcon(nullptr),
      trayMenu(nullptr),
      updateTimer(nullptr),
      lastPercentage(-1),
      lastCharging(false),
      lastEnergyRate(0.0),
      lowBatteryWarningShown(false),
      criticalBatteryWarningShown(false),
      lowBatteryThreshold(20),
      criticalBatteryThreshold(5) {
    loadSystemSettings();
    findBatteryDevice();
    createMenu();
    createTrayIcon();

    // Set up update timer
    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &BatteryTray::updateBattery);
    updateTimer->start(UPDATE_INTERVAL_MS);

    // Initial update
    updateBattery();

    trayIcon->show();
}

BatteryTray::~BatteryTray() {}

void BatteryTray::loadSystemSettings() {
    auto config = UPowerHelper::readPowerConfig();
    lowBatteryThreshold = config.percentageLow;
    criticalBatteryThreshold = config.percentageCritical;
}

void BatteryTray::findBatteryDevice() {
    batteryDevicePath = UPowerHelper::findBatteryDevice();
}

void BatteryTray::createMenu() {
    trayMenu = new QMenu();

    QAction *settingsAction = trayMenu->addAction(tr("Power Settings..."));
    connect(settingsAction, &QAction::triggered, this, &BatteryTray::showSettings);

    trayMenu->addSeparator();

    QAction *quitAction = trayMenu->addAction(tr("Quit"));
    connect(quitAction, &QAction::triggered, this, &BatteryTray::quit);
}

void BatteryTray::createTrayIcon() {
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayMenu);

    connect(trayIcon, &QSystemTrayIcon::activated, this, &BatteryTray::onActivated);
}

void BatteryTray::updateBattery() {
    auto batteryInfo = UPowerHelper::getBatteryInfo(batteryDevicePath);

    if (!batteryInfo.has_value()) {
        trayIcon->setToolTip(tr("Battery: Unknown (UPower not available)"));
        return;
    }

    int percentage = batteryInfo->percentage;
    bool charging = batteryInfo->isCharging();
    bool fullyCharged = batteryInfo->isFullyCharged();

    double energyRate = batteryInfo->energyRate;

    QString stateStr;
    if (fullyCharged) {
        stateStr = tr("Fully charged");
    } else if (charging) {
        stateStr = tr("Charging");
    } else {
        stateStr = tr("Discharging");
    }

    // Update icon (both tray and taskbar)
    BatteryIcon::Thresholds thresholds{lowBatteryThreshold, criticalBatteryThreshold};
    QIcon batteryIcon = BatteryIcon::create(22, percentage, charging || fullyCharged, thresholds);
    trayIcon->setIcon(batteryIcon);
    qApp->setWindowIcon(batteryIcon);

    // Get power profile and format display name
    QString powerProfile = UPowerHelper::getActivePowerProfile();
    if (!powerProfile.isEmpty() && powerProfile != "Unknown") {
        QStringList parts = powerProfile.split(QLatin1Char('-'), Qt::SkipEmptyParts);
        for (QString &part : parts) {
            if (!part.isEmpty()) {
                part[0] = part[0].toUpper();
            }
        }
        powerProfile = parts.join(QLatin1Char(' '));
    }

    // Update tooltip
    QString batteryLine;
    QString powerLine;

    if (fullyCharged) {
        batteryLine = tr("Battery: %1% (Fully charged)").arg(percentage);
        powerLine = tr("Power: %1").arg(powerProfile);
    } else if (charging) {
        // When charging, show charge rate on battery line
        QString rateStr;
        if (energyRate > 0.001) {
            rateStr = tr(" @ %1 W").arg(energyRate, 0, 'f', 1);
        }
        batteryLine = tr("Battery: %1% (Charging%2)").arg(percentage).arg(rateStr);
        powerLine = tr("Power: %1").arg(powerProfile);
    } else {
        // When discharging, show power consumption on power line
        batteryLine = tr("Battery: %1% (Discharging)").arg(percentage);
        if (energyRate > 0.001) {
            powerLine = tr("Power: %1 (%2 W)").arg(powerProfile).arg(energyRate, 0, 'f', 1);
        } else {
            powerLine = tr("Power: %1").arg(powerProfile);
        }
    }

    trayIcon->setToolTip(batteryLine + "\n" + powerLine);

    // Check for low battery warnings (only when discharging)
    if (!charging && !fullyCharged) {
        if (percentage <= criticalBatteryThreshold && !criticalBatteryWarningShown) {
            showLowBatteryNotification(percentage);
            criticalBatteryWarningShown = true;
        } else if (percentage <= lowBatteryThreshold && !lowBatteryWarningShown) {
            showLowBatteryNotification(percentage);
            lowBatteryWarningShown = true;
        }
    }

    // Reset warning flags when charging or battery recovers
    if (charging || percentage > lowBatteryThreshold) {
        lowBatteryWarningShown = false;
    }
    if (charging || percentage > criticalBatteryThreshold) {
        criticalBatteryWarningShown = false;
    }

    lastPercentage = percentage;
    lastCharging = charging || fullyCharged;
    lastEnergyRate = energyRate;
}

void BatteryTray::showLowBatteryNotification(int percentage) {
    QString title;
    QString message;
    QSystemTrayIcon::MessageIcon icon;

    if (percentage <= criticalBatteryThreshold) {
        title = tr("Critical Battery Warning");
        message = tr("Battery level is critically low at %1%!\nPlug in your charger immediately.")
                      .arg(percentage);
        icon = QSystemTrayIcon::Critical;
    } else {
        title = tr("Low Battery Warning");
        message =
            tr("Battery level is low at %1%.\nConsider plugging in your charger.").arg(percentage);
        icon = QSystemTrayIcon::Warning;
    }

    trayIcon->showMessage(title, message, icon, 10000);
}

void BatteryTray::onActivated(QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::Trigger) {
        // Left click - open settings
        showSettings();
    }
}

void BatteryTray::quit() {
    QApplication::quit();
}

void BatteryTray::showSettings() {
    SettingsDialog dialog(lastPercentage, lastCharging, lastEnergyRate);
    dialog.exec();
}
