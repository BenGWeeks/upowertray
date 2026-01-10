#include "settingsdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QFrame>
#include <QPainter>
#include <QFile>
#include <QTextStream>
#include <QGroupBox>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusConnection>
#include <QDBusVariant>

SettingsDialog::SettingsDialog(int batteryPercent, bool charging, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("upowertray - Power Settings");
    setMinimumWidth(400);

    auto *layout = new QVBoxLayout(this);

    // Read system settings first (need critical threshold for icon)
    SystemPowerSettings sys = readSystemSettings();

    // Battery status section
    auto *batteryFrame = new QFrame(this);
    batteryFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
    auto *batteryLayout = new QHBoxLayout(batteryFrame);

    batteryLayout->addStretch();  // Center the content

    batteryIconLabel = new QLabel(this);
    QIcon batteryIcon = createBatteryIcon(batteryPercent, charging,
        static_cast<int>(sys.percentageLow), static_cast<int>(sys.percentageCritical));
    batteryIconLabel->setPixmap(batteryIcon.pixmap(64, 64));
    batteryLayout->addWidget(batteryIconLabel);

    QString statusText = QString("<b style='font-size: 24px;'>%1%</b><br><span style='font-size: 14px;'>%2</span>")
        .arg(batteryPercent)
        .arg(charging ? "Charging" : "Discharging");
    batteryTextLabel = new QLabel(statusText, this);
    batteryLayout->addWidget(batteryTextLabel);

    batteryLayout->addStretch();  // Center the content

    layout->addWidget(batteryFrame);
    layout->addSpacing(10);

    // UPower notification thresholds
    auto *notifyGroup = new QGroupBox("Notification Thresholds (UPower)", this);
    auto *notifyLayout = new QFormLayout(notifyGroup);

    auto *lowLabel = new QLabel(QString("<b>%1%</b>").arg(sys.percentageLow, 0, 'f', 0), this);
    notifyLayout->addRow("Low battery warning:", lowLabel);

    auto *critLabel = new QLabel(QString("<b>%1%</b>").arg(sys.percentageCritical, 0, 'f', 0), this);
    notifyLayout->addRow("Critical battery warning:", critLabel);

    layout->addWidget(notifyGroup);

    // Battery critical action
    auto *actionGroup = new QGroupBox("Battery Critical Action (UPower)", this);
    auto *actionLayout = new QFormLayout(actionGroup);

    auto *actionLabel = new QLabel(QString("<b>%1</b> at %2%")
        .arg(sys.criticalAction)
        .arg(sys.percentageAction, 0, 'f', 0), this);
    actionLayout->addRow("Action:", actionLabel);

    layout->addWidget(actionGroup);

    // Lid close action
    auto *lidGroup = new QGroupBox("Lid Close Action (systemd-logind)", this);
    auto *lidLayout = new QFormLayout(lidGroup);

    auto *lidBatteryLabel = new QLabel(QString("<b>%1</b>").arg(sys.handleLidSwitch), this);
    lidLayout->addRow("On battery:", lidBatteryLabel);

    auto *lidPowerLabel = new QLabel(QString("<b>%1</b>").arg(sys.handleLidSwitchExternalPower), this);
    lidLayout->addRow("On AC power:", lidPowerLabel);

    layout->addWidget(lidGroup);

    // Power profile
    auto *profileGroup = new QGroupBox("Power Profile (power-profiles-daemon)", this);
    auto *profileLayout = new QFormLayout(profileGroup);

    profileCombo = new QComboBox(this);
    QStringList profiles = getAvailablePowerProfiles();
    for (const QString &profile : profiles) {
        QString displayName = profile;
        displayName[0] = displayName[0].toUpper();
        displayName.replace("-", " ");
        profileCombo->addItem(displayName, profile);  // Display name, actual value
    }
    // Select current profile
    int currentIndex = profiles.indexOf(sys.powerProfile);
    if (currentIndex >= 0) {
        profileCombo->setCurrentIndex(currentIndex);
    }
    connect(profileCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SettingsDialog::onPowerProfileChanged);
    profileLayout->addRow("Current profile:", profileCombo);

    layout->addWidget(profileGroup);

    // Close button
    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttonBox);

    setLayout(layout);
}

SystemPowerSettings SettingsDialog::readSystemSettings()
{
    SystemPowerSettings settings;

    // Read UPower config
    QFile upowerFile("/etc/UPower/UPower.conf");
    if (upowerFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&upowerFile);
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (line.startsWith('#') || line.isEmpty()) continue;

            if (line.startsWith("PercentageLow=")) {
                settings.percentageLow = line.mid(14).toDouble();
            } else if (line.startsWith("PercentageCritical=")) {
                settings.percentageCritical = line.mid(19).toDouble();
            } else if (line.startsWith("PercentageAction=")) {
                settings.percentageAction = line.mid(17).toDouble();
            } else if (line.startsWith("CriticalPowerAction=")) {
                settings.criticalAction = line.mid(20);
            }
        }
        upowerFile.close();
    }

    // Read logind config
    QFile logindFile("/etc/systemd/logind.conf");
    if (logindFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&logindFile);
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (line.startsWith('#') || line.isEmpty()) continue;

            if (line.startsWith("HandleLidSwitch=")) {
                settings.handleLidSwitch = line.mid(16);
            } else if (line.startsWith("HandleLidSwitchExternalPower=")) {
                settings.handleLidSwitchExternalPower = line.mid(29);
            }
        }
        logindFile.close();
    }

    // Read power profile via DBus (store raw value like "performance")
    QDBusInterface powerProfiles(
        "net.hadess.PowerProfiles",
        "/net/hadess/PowerProfiles",
        "org.freedesktop.DBus.Properties",
        QDBusConnection::systemBus()
    );
    if (powerProfiles.isValid()) {
        QDBusReply<QVariant> reply = powerProfiles.call(
            "Get", "net.hadess.PowerProfiles", "ActiveProfile"
        );
        if (reply.isValid()) {
            settings.powerProfile = reply.value().toString();
        }
    }

    return settings;
}

QIcon SettingsDialog::createBatteryIcon(int percentage, bool charging, int lowThreshold, int criticalThreshold)
{
    const int size = 64;
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // Battery body dimensions (scaled for 64px)
    const int bodyLeft = 6;
    const int bodyTop = 10;
    const int bodyWidth = 44;
    const int bodyHeight = 44;
    const int tipWidth = 6;
    const int tipHeight = 18;

    // Outline color - white normally, amber at low, red at critical
    QColor outlineColor;
    if (percentage <= criticalThreshold) {
        outlineColor = QColor(220, 50, 50);  // Red
    } else if (percentage <= lowThreshold) {
        outlineColor = QColor(220, 180, 50); // Yellow/Orange
    } else {
        outlineColor = Qt::white;  // White when healthy
    }

    // Draw battery outline
    painter.setPen(QPen(outlineColor, 2.5));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(bodyLeft, bodyTop, bodyWidth, bodyHeight, 4, 4);

    // Draw battery tip (positive terminal)
    painter.fillRect(bodyLeft + bodyWidth, bodyTop + (bodyHeight - tipHeight) / 2,
                     tipWidth, tipHeight, outlineColor);

    // Calculate fill width based on percentage
    const int fillMargin = 3;
    const int maxFillWidth = bodyWidth - 2 * fillMargin;
    const int fillWidth = (percentage * maxFillWidth) / 100;
    const int fillHeight = bodyHeight - 2 * fillMargin;

    // Choose fill color based on percentage (same thresholds as outline)
    QColor fillColor;
    if (percentage <= criticalThreshold) {
        fillColor = QColor(220, 50, 50);  // Red
    } else if (percentage <= lowThreshold) {
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
        bolt << QPoint(34, 12)
             << QPoint(20, 32)
             << QPoint(28, 32)
             << QPoint(20, 52)
             << QPoint(40, 28)
             << QPoint(30, 28);
        painter.drawPolygon(bolt);
    }

    painter.end();
    return QIcon(pixmap);
}

QStringList SettingsDialog::getAvailablePowerProfiles()
{
    // Standard power-profiles-daemon profiles
    return QStringList() << "power-saver" << "balanced" << "performance";
}

void SettingsDialog::setPowerProfile(const QString &profile)
{
    QDBusInterface powerProfiles(
        "net.hadess.PowerProfiles",
        "/net/hadess/PowerProfiles",
        "org.freedesktop.DBus.Properties",
        QDBusConnection::systemBus()
    );

    if (powerProfiles.isValid()) {
        powerProfiles.call("Set", "net.hadess.PowerProfiles", "ActiveProfile",
                          QVariant::fromValue(QDBusVariant(profile)));
    }
}

void SettingsDialog::onPowerProfileChanged(int index)
{
    QString profile = profileCombo->itemData(index).toString();
    setPowerProfile(profile);
}
