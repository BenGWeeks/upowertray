#include "settingsdialog.h"
#include "batteryicon.h"
#include "upowerhelper.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QFrame>
#include <QGroupBox>
#include <QDesktopServices>
#include <QUrl>
#include <QProcess>
#include <QStandardPaths>

SettingsDialog::SettingsDialog(int batteryPercent, bool charging, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("upowertray - Power Settings"));
    setMinimumWidth(400);

    auto *layout = new QVBoxLayout(this);

    // Read system settings
    auto config = UPowerHelper::readPowerConfig();
    int lowThreshold = config.percentageLow;
    int criticalThreshold = config.percentageCritical;

    // Battery status section
    auto *batteryFrame = new QFrame(this);
    batteryFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
    auto *batteryLayout = new QHBoxLayout(batteryFrame);

    batteryLayout->addStretch();  // Center the content

    batteryIconLabel = new QLabel(this);
    BatteryIcon::Thresholds thresholds{lowThreshold, criticalThreshold};
    QIcon batteryIcon = BatteryIcon::create(64, batteryPercent, charging, thresholds);
    batteryIconLabel->setPixmap(batteryIcon.pixmap(64, 64));
    batteryLayout->addWidget(batteryIconLabel);

    QString statusText = QString("<b style='font-size: 24px;'>%1%</b><br><span style='font-size: 14px;'>%2</span>")
        .arg(batteryPercent)
        .arg(charging ? tr("Charging") : tr("Discharging"));
    batteryTextLabel = new QLabel(statusText, this);
    batteryLayout->addWidget(batteryTextLabel);

    batteryLayout->addStretch();  // Center the content

    layout->addWidget(batteryFrame);
    layout->addSpacing(10);

    // UPower notification thresholds
    auto *notifyGroup = new QGroupBox(tr("Notification Thresholds (UPower)"), this);
    auto *notifyLayout = new QFormLayout(notifyGroup);

    auto *lowLabel = new QLabel(QString("<b>%1%</b>").arg(config.percentageLow), this);
    notifyLayout->addRow(tr("Low battery warning:"), lowLabel);

    auto *critLabel = new QLabel(QString("<b>%1%</b>").arg(config.percentageCritical), this);
    notifyLayout->addRow(tr("Critical battery warning:"), critLabel);

    layout->addWidget(notifyGroup);

    // Battery critical action
    auto *actionGroup = new QGroupBox(tr("Battery Critical Action (UPower)"), this);
    auto *actionLayout = new QFormLayout(actionGroup);

    auto *actionLabel = new QLabel(QString("<b>%1</b> at %2%")
        .arg(config.criticalAction)
        .arg(config.percentageAction), this);
    actionLayout->addRow(tr("Action:"), actionLabel);

    layout->addWidget(actionGroup);

    // Lid close action
    auto *lidGroup = new QGroupBox(tr("Lid Close Action (systemd-logind)"), this);
    auto *lidLayout = new QFormLayout(lidGroup);

    auto *lidBatteryLabel = new QLabel(QString("<b>%1</b>").arg(config.handleLidSwitch), this);
    lidLayout->addRow(tr("On battery:"), lidBatteryLabel);

    auto *lidPowerLabel = new QLabel(QString("<b>%1</b>").arg(config.handleLidSwitchExternalPower), this);
    lidLayout->addRow(tr("On AC power:"), lidPowerLabel);

    layout->addWidget(lidGroup);

    // Power profile
    auto *profileGroup = new QGroupBox(tr("Power Profile (power-profiles-daemon)"), this);
    auto *profileLayout = new QFormLayout(profileGroup);

    profileCombo = new QComboBox(this);
    QStringList profiles = UPowerHelper::getAvailablePowerProfiles();
    QString currentProfile = UPowerHelper::getActivePowerProfile();

    for (const QString &profile : profiles) {
        QString displayName = profile;
        if (!displayName.isEmpty()) {
            displayName[0] = displayName[0].toUpper();
            displayName.replace("-", " ");
        }
        profileCombo->addItem(displayName, profile);  // Display name, actual value
    }

    // Select current profile
    int currentIndex = profiles.indexOf(currentProfile);
    if (currentIndex >= 0) {
        profileCombo->setCurrentIndex(currentIndex);
    }
    connect(profileCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SettingsDialog::onPowerProfileChanged);
    profileLayout->addRow(tr("Current profile:"), profileCombo);

    layout->addWidget(profileGroup);

    // Button row with config editor, GitHub link and Close button
    auto *buttonLayout = new QHBoxLayout();

    auto *configButton = new QPushButton(this);
    configButton->setIcon(QIcon::fromTheme("configure", QIcon::fromTheme("preferences-system")));
    configButton->setText(tr("Edit Config..."));
    configButton->setFlat(true);
    connect(configButton, &QPushButton::clicked, this, &SettingsDialog::openConfigEditor);
    buttonLayout->addWidget(configButton);

    auto *githubButton = new QPushButton(this);
    githubButton->setIcon(QIcon::fromTheme("web-browser", QIcon::fromTheme("help-about")));
    githubButton->setText(tr("Report Issue..."));
    githubButton->setFlat(true);
    connect(githubButton, &QPushButton::clicked, this, &SettingsDialog::openGitHubIssues);
    buttonLayout->addWidget(githubButton);

    buttonLayout->addStretch();

    auto *closeButton = new QPushButton(tr("Close"), this);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::reject);
    buttonLayout->addWidget(closeButton);

    layout->addLayout(buttonLayout);

    setLayout(layout);
}

void SettingsDialog::onPowerProfileChanged(int index)
{
    QString profile = profileCombo->itemData(index).toString();
    UPowerHelper::setActivePowerProfile(profile);
}

void SettingsDialog::openGitHubIssues()
{
    QDesktopServices::openUrl(QUrl("https://github.com/BenGWeeks/upowertray/issues"));
}

void SettingsDialog::openConfigEditor()
{
    // Config files to edit
    QStringList configFiles = {
        "/etc/systemd/logind.conf",
        "/etc/UPower/UPower.conf"
    };

    // Try to find an available text editor (prefer KDE/Qt editors)
    QStringList editors = {"kate", "kwrite", "gedit", "xed", "featherpad", "mousepad", "nano"};
    QString editor;

    for (const QString &e : editors) {
        QString path = QStandardPaths::findExecutable(e);
        if (!path.isEmpty()) {
            editor = path;
            break;
        }
    }

    if (editor.isEmpty()) {
        editor = "nano";  // Fallback to nano (usually available)
    }

    // Use pkexec for elevated privileges
    QStringList args = {editor};
    args.append(configFiles);
    QProcess::startDetached("pkexec", args);
}
