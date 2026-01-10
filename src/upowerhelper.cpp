#include "upowerhelper.h"

#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusReply>
#include <QDBusVariant>
#include <QFile>
#include <QRegularExpression>
#include <QTextStream>

const QString UPowerHelper::UPOWER_SERVICE = "org.freedesktop.UPower";
const QString UPowerHelper::UPOWER_PATH = "/org/freedesktop/UPower";
const QString UPowerHelper::POWER_PROFILES_SERVICE = "net.hadess.PowerProfiles";
const QString UPowerHelper::POWER_PROFILES_PATH = "/net/hadess/PowerProfiles";

QString UPowerHelper::findBatteryDevice() {
    QDBusInterface upower(UPOWER_SERVICE, UPOWER_PATH, "org.freedesktop.UPower",
                          QDBusConnection::systemBus());

    if (!upower.isValid()) {
        return QString();
    }

    QDBusReply<QList<QDBusObjectPath>> reply = upower.call("EnumerateDevices");
    if (!reply.isValid()) {
        return QString();
    }

    for (const QDBusObjectPath &path : reply.value()) {
        QString devicePath = path.path();

        // Check if this is a battery device
        QDBusInterface device(UPOWER_SERVICE, devicePath, "org.freedesktop.DBus.Properties",
                              QDBusConnection::systemBus());

        QDBusReply<QVariant> typeReply =
            device.call("Get", "org.freedesktop.UPower.Device", "Type");
        if (typeReply.isValid()) {
            // Type 2 = Battery
            if (typeReply.value().toUInt() == 2) {
                // Verify it's a real battery (not a peripheral)
                QDBusReply<QVariant> powerSupplyReply =
                    device.call("Get", "org.freedesktop.UPower.Device", "PowerSupply");
                if (powerSupplyReply.isValid() && powerSupplyReply.value().toBool()) {
                    return devicePath;
                }
            }
        }
    }

    // Fallback to common paths
    QStringList fallbackPaths = {"/org/freedesktop/UPower/devices/battery_BAT0",
                                 "/org/freedesktop/UPower/devices/battery_BAT1",
                                 "/org/freedesktop/UPower/devices/battery_CMB0"};

    for (const QString &path : fallbackPaths) {
        QDBusInterface device(UPOWER_SERVICE, path, "org.freedesktop.DBus.Properties",
                              QDBusConnection::systemBus());
        if (device.isValid()) {
            QDBusReply<QVariant> reply =
                device.call("Get", "org.freedesktop.UPower.Device", "IsPresent");
            if (reply.isValid() && reply.value().toBool()) {
                return path;
            }
        }
    }

    return QString();
}

std::optional<UPowerHelper::BatteryInfo> UPowerHelper::getBatteryInfo(const QString &devicePath) {
    QString path = devicePath.isEmpty() ? findBatteryDevice() : devicePath;
    if (path.isEmpty()) {
        return std::nullopt;
    }

    QDBusInterface device(UPOWER_SERVICE, path, "org.freedesktop.DBus.Properties",
                          QDBusConnection::systemBus());

    if (!device.isValid()) {
        return std::nullopt;
    }

    BatteryInfo info;
    info.devicePath = path;

    QDBusReply<QVariant> percentReply =
        device.call("Get", "org.freedesktop.UPower.Device", "Percentage");
    if (percentReply.isValid()) {
        info.percentage = static_cast<int>(percentReply.value().toDouble());
    }

    QDBusReply<QVariant> stateReply = device.call("Get", "org.freedesktop.UPower.Device", "State");
    if (stateReply.isValid()) {
        info.state = stateReply.value().toUInt();
    }

    QDBusReply<QVariant> presentReply =
        device.call("Get", "org.freedesktop.UPower.Device", "IsPresent");
    if (presentReply.isValid()) {
        info.isPresent = presentReply.value().toBool();
    }

    return info;
}

QStringList UPowerHelper::getAvailablePowerProfiles() {
    QStringList profiles;

    QDBusInterface powerProfiles(POWER_PROFILES_SERVICE, POWER_PROFILES_PATH,
                                 "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());

    if (!powerProfiles.isValid()) {
        // Fallback to standard profiles if daemon not available
        return QStringList() << "power-saver"
                             << "balanced"
                             << "performance";
    }

    QDBusReply<QVariant> reply = powerProfiles.call("Get", "net.hadess.PowerProfiles", "Profiles");
    if (!reply.isValid()) {
        return QStringList() << "power-saver"
                             << "balanced"
                             << "performance";
    }

    // Parse the array of dictionaries
    QDBusArgument arg = reply.value().value<QDBusArgument>();
    arg.beginArray();
    while (!arg.atEnd()) {
        arg.beginMap();
        while (!arg.atEnd()) {
            QString key;
            QVariant value;
            arg.beginMapEntry();
            arg >> key >> value;
            arg.endMapEntry();
            if (key == "Profile") {
                profiles << value.toString();
            }
        }
        arg.endMap();
    }
    arg.endArray();

    if (profiles.isEmpty()) {
        return QStringList() << "power-saver"
                             << "balanced"
                             << "performance";
    }

    return profiles;
}

QString UPowerHelper::getActivePowerProfile() {
    QDBusInterface powerProfiles(POWER_PROFILES_SERVICE, POWER_PROFILES_PATH,
                                 "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());

    if (!powerProfiles.isValid()) {
        return QObject::tr("Unknown");
    }

    QDBusReply<QVariant> reply =
        powerProfiles.call("Get", "net.hadess.PowerProfiles", "ActiveProfile");
    if (!reply.isValid()) {
        return QObject::tr("Unknown");
    }

    return reply.value().toString();
}

bool UPowerHelper::setActivePowerProfile(const QString &profile) {
    QDBusInterface powerProfiles(POWER_PROFILES_SERVICE, POWER_PROFILES_PATH,
                                 "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());

    if (!powerProfiles.isValid()) {
        return false;
    }

    QDBusMessage reply = powerProfiles.call("Set", "net.hadess.PowerProfiles", "ActiveProfile",
                                            QVariant::fromValue(QDBusVariant(profile)));

    return reply.type() != QDBusMessage::ErrorMessage;
}

UPowerHelper::PowerConfig UPowerHelper::readPowerConfig() {
    PowerConfig config;

    // Regular expressions for parsing config files
    static const QRegularExpression percentageLowRe("^PercentageLow\\s*=\\s*(\\d+(?:\\.\\d+)?)");
    static const QRegularExpression percentageCriticalRe(
        "^PercentageCritical\\s*=\\s*(\\d+(?:\\.\\d+)?)");
    static const QRegularExpression percentageActionRe(
        "^PercentageAction\\s*=\\s*(\\d+(?:\\.\\d+)?)");
    static const QRegularExpression criticalActionRe("^CriticalPowerAction\\s*=\\s*(\\w+)");
    static const QRegularExpression handleLidSwitchRe("^HandleLidSwitch\\s*=\\s*(\\w+)");
    static const QRegularExpression handleLidSwitchExternalRe(
        "^HandleLidSwitchExternalPower\\s*=\\s*(\\w+)");

    // Read UPower config
    QFile upowerFile("/etc/UPower/UPower.conf");
    if (upowerFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&upowerFile);
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (line.startsWith('#') || line.isEmpty())
                continue;

            QRegularExpressionMatch match;

            if ((match = percentageLowRe.match(line)).hasMatch()) {
                config.percentageLow = static_cast<int>(match.captured(1).toDouble());
            } else if ((match = percentageCriticalRe.match(line)).hasMatch()) {
                config.percentageCritical = static_cast<int>(match.captured(1).toDouble());
            } else if ((match = percentageActionRe.match(line)).hasMatch()) {
                config.percentageAction = static_cast<int>(match.captured(1).toDouble());
            } else if ((match = criticalActionRe.match(line)).hasMatch()) {
                config.criticalAction = match.captured(1);
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
            if (line.startsWith('#') || line.isEmpty())
                continue;

            QRegularExpressionMatch match;

            if ((match = handleLidSwitchRe.match(line)).hasMatch()) {
                config.handleLidSwitch = match.captured(1);
            } else if ((match = handleLidSwitchExternalRe.match(line)).hasMatch()) {
                config.handleLidSwitchExternalPower = match.captured(1);
            }
        }
        logindFile.close();
    }

    return config;
}
