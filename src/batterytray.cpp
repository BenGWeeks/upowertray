#include "batterytray.h"
#include "settingsdialog.h"
#include <QPainter>
#include <QDBusReply>
#include <QApplication>
#include <QFile>
#include <QTextStream>

BatteryTray::BatteryTray(QObject *parent)
    : QObject(parent)
    , trayIcon(nullptr)
    , trayMenu(nullptr)
    , updateTimer(nullptr)
    , upowerDevice(nullptr)
    , lastPercentage(-1)
    , lastCharging(false)
    , lowBatteryWarningShown(false)
    , criticalBatteryWarningShown(false)
    , lowBatteryThreshold(20)
    , criticalBatteryThreshold(5)
{
    loadSystemSettings();
    createMenu();
    createTrayIcon();

    // Set up UPower DBus interface
    upowerDevice = new QDBusInterface(
        "org.freedesktop.UPower",
        "/org/freedesktop/UPower/devices/battery_BAT0",
        "org.freedesktop.DBus.Properties",
        QDBusConnection::systemBus(),
        this
    );

    // Set up power profiles DBus interface
    powerProfiles = new QDBusInterface(
        "net.hadess.PowerProfiles",
        "/net/hadess/PowerProfiles",
        "org.freedesktop.DBus.Properties",
        QDBusConnection::systemBus(),
        this
    );

    // Set up update timer
    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &BatteryTray::updateBattery);
    updateTimer->start(UPDATE_INTERVAL_MS);

    // Initial update
    updateBattery();

    trayIcon->show();
}

BatteryTray::~BatteryTray()
{
}

void BatteryTray::loadSystemSettings()
{
    QFile file("/etc/UPower/UPower.conf");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (line.startsWith('#') || line.isEmpty()) continue;

            if (line.startsWith("PercentageLow=")) {
                lowBatteryThreshold = static_cast<int>(line.mid(14).toDouble());
            } else if (line.startsWith("PercentageCritical=")) {
                criticalBatteryThreshold = static_cast<int>(line.mid(19).toDouble());
            }
        }
        file.close();
    }
}

void BatteryTray::createMenu()
{
    trayMenu = new QMenu();

    QAction *settingsAction = trayMenu->addAction("Power Settings...");
    connect(settingsAction, &QAction::triggered, this, &BatteryTray::showSettings);

    trayMenu->addSeparator();

    QAction *quitAction = trayMenu->addAction("Quit");
    connect(quitAction, &QAction::triggered, this, &BatteryTray::quit);
}

void BatteryTray::createTrayIcon()
{
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayMenu);

    connect(trayIcon, &QSystemTrayIcon::activated,
            this, &BatteryTray::onActivated);
}

void BatteryTray::updateBattery()
{
    if (!upowerDevice || !upowerDevice->isValid()) {
        trayIcon->setToolTip("Battery: Unknown (UPower not available)");
        return;
    }

    // Get percentage
    QDBusReply<QVariant> percentReply = upowerDevice->call(
        "Get", "org.freedesktop.UPower.Device", "Percentage"
    );

    // Get state
    QDBusReply<QVariant> stateReply = upowerDevice->call(
        "Get", "org.freedesktop.UPower.Device", "State"
    );

    if (!percentReply.isValid() || !stateReply.isValid()) {
        trayIcon->setToolTip("Battery: Error reading status");
        return;
    }

    int percentage = static_cast<int>(percentReply.value().toDouble());
    uint state = stateReply.value().toUInt();

    // State: 1=charging, 2=discharging, 3=empty, 4=fully-charged, 5=pending-charge, 6=pending-discharge
    bool charging = (state == 1 || state == 5);
    bool fullyCharged = (state == 4);

    QString stateStr;
    if (fullyCharged) {
        stateStr = "Fully charged";
    } else if (charging) {
        stateStr = "Charging";
    } else {
        stateStr = "Discharging";
    }

    // Update icon (both tray and taskbar)
    QIcon batteryIcon = createBatteryIcon(percentage, charging || fullyCharged);
    trayIcon->setIcon(batteryIcon);
    qApp->setWindowIcon(batteryIcon);

    // Get power profile
    QString powerProfile = "Unknown";
    if (powerProfiles && powerProfiles->isValid()) {
        QDBusReply<QVariant> profileReply = powerProfiles->call(
            "Get", "net.hadess.PowerProfiles", "ActiveProfile"
        );
        if (profileReply.isValid()) {
            QString profile = profileReply.value().toString();
            // Capitalize first letter
            if (!profile.isEmpty()) {
                profile[0] = profile[0].toUpper();
                profile.replace("-", " ");
                powerProfile = profile;
            }
        }
    }

    // Update tooltip
    trayIcon->setToolTip(QString("Battery: %1% (%2)\nPower: %3").arg(percentage).arg(stateStr).arg(powerProfile));

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
}

QIcon BatteryTray::createBatteryIcon(int percentage, bool charging)
{
    const int size = 22;
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // Battery body dimensions
    const int bodyLeft = 2;
    const int bodyTop = 4;
    const int bodyWidth = 16;
    const int bodyHeight = 14;
    const int tipWidth = 2;
    const int tipHeight = 6;

    // Outline color - white normally, amber at low, red at critical
    QColor outlineColor;
    if (percentage <= criticalBatteryThreshold) {
        outlineColor = QColor(220, 50, 50);  // Red
    } else if (percentage <= lowBatteryThreshold) {
        outlineColor = QColor(220, 180, 50); // Yellow/Orange
    } else {
        outlineColor = Qt::white;  // White when healthy
    }

    // Draw battery outline
    painter.setPen(QPen(outlineColor, 1.5));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(bodyLeft, bodyTop, bodyWidth, bodyHeight, 2, 2);

    // Draw battery tip (positive terminal)
    painter.fillRect(bodyLeft + bodyWidth, bodyTop + (bodyHeight - tipHeight) / 2,
                     tipWidth, tipHeight, outlineColor);

    // Calculate fill width based on percentage
    const int fillMargin = 2;
    const int maxFillWidth = bodyWidth - 2 * fillMargin;
    const int fillWidth = (percentage * maxFillWidth) / 100;
    const int fillHeight = bodyHeight - 2 * fillMargin;

    // Choose fill color based on percentage (same thresholds as outline)
    QColor fillColor;
    if (percentage <= criticalBatteryThreshold) {
        fillColor = QColor(220, 50, 50);  // Red
    } else if (percentage <= lowBatteryThreshold) {
        fillColor = QColor(220, 180, 50); // Yellow/Orange
    } else {
        fillColor = QColor(50, 200, 50);  // Green
    }

    // Draw fill
    painter.fillRect(bodyLeft + fillMargin, bodyTop + fillMargin,
                     fillWidth, fillHeight, fillColor);

    // Draw charging indicator (lightning bolt)
    if (charging) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::white);

        QPolygon bolt;
        bolt << QPoint(12, 5)
             << QPoint(8, 11)
             << QPoint(10, 11)
             << QPoint(8, 17)
             << QPoint(14, 10)
             << QPoint(11, 10);
        painter.drawPolygon(bolt);
    }

    painter.end();
    return QIcon(pixmap);
}

void BatteryTray::showLowBatteryNotification(int percentage)
{
    QString title;
    QString message;
    QSystemTrayIcon::MessageIcon icon;

    if (percentage <= criticalBatteryThreshold) {
        title = "Critical Battery Warning";
        message = QString("Battery level is critically low at %1%!\nPlug in your charger immediately.").arg(percentage);
        icon = QSystemTrayIcon::Critical;
    } else {
        title = "Low Battery Warning";
        message = QString("Battery level is low at %1%.\nConsider plugging in your charger.").arg(percentage);
        icon = QSystemTrayIcon::Warning;
    }

    trayIcon->showMessage(title, message, icon, 10000);
}

void BatteryTray::onActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger) {
        // Left click - open settings
        showSettings();
    }
}

void BatteryTray::quit()
{
    QApplication::quit();
}

void BatteryTray::showSettings()
{
    SettingsDialog dialog(lastPercentage, lastCharging);
    dialog.exec();
}
