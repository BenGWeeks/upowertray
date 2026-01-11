#ifndef UPOWERHELPER_H
#define UPOWERHELPER_H

#include <QDBusInterface>
#include <QString>
#include <QStringList>

#include <optional>

/**
 * Helper class for UPower and power-profiles-daemon DBus interactions.
 * Provides dynamic device detection and error handling.
 */
class UPowerHelper {
public:
    struct BatteryInfo {
        QString devicePath;
        int percentage = 0;
        uint state = 0;  // 1=charging, 2=discharging, 3=empty, 4=fully-charged, 5=pending-charge
        bool isPresent = false;

        bool isCharging() const { return state == 1 || state == 5; }
        bool isFullyCharged() const { return state == 4; }
        bool isDischarging() const { return state == 2; }
    };

    struct PowerConfig {
        int percentageLow = 20;
        int percentageCritical = 5;
        int percentageAction = 2;
        QString criticalAction = "HybridSleep";
        QString handleLidSwitch = "suspend";
        QString handleLidSwitchExternalPower = "suspend";
    };

    /**
     * Find the first available battery device path.
     * @return Device path or empty string if no battery found
     */
    static QString findBatteryDevice();

    /**
     * Get battery information from UPower.
     * @param devicePath The DBus device path (or empty to auto-detect)
     * @return BatteryInfo struct or nullopt on error
     */
    static std::optional<BatteryInfo> getBatteryInfo(const QString &devicePath = QString());

    /**
     * Get available power profiles from power-profiles-daemon.
     * @return List of profile names (e.g., "power-saver", "balanced", "performance")
     */
    static QStringList getAvailablePowerProfiles();

    /**
     * Get the current active power profile.
     * @return Profile name or "Unknown" on error
     */
    static QString getActivePowerProfile();

    /**
     * Set the active power profile.
     * @param profile Profile name to set
     * @return true on success, false on error
     */
    static bool setActivePowerProfile(const QString &profile);

    /**
     * Read power configuration from system files.
     * @return PowerConfig with values from /etc/UPower/UPower.conf and /etc/systemd/logind.conf
     */
    static PowerConfig readPowerConfig();

private:
    static const QString UPOWER_SERVICE;
    static const QString UPOWER_PATH;
    static const QString POWER_PROFILES_SERVICE;
    static const QString POWER_PROFILES_PATH;
};

#endif  // UPOWERHELPER_H
