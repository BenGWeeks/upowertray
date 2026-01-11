#include <QtTest/QtTest>
#include "../src/upowerhelper.h"

class TestUPowerHelper : public QObject
{
    Q_OBJECT

private slots:
    void testBatteryInfo_defaultValues();
    void testBatteryInfo_isCharging();
    void testBatteryInfo_isFullyCharged();
    void testBatteryInfo_isDischarging();
    void testPowerConfig_defaultValues();
    void testReadPowerConfig_returnsValidStruct();
    void testGetAvailablePowerProfiles_notEmpty();
};

void TestUPowerHelper::testBatteryInfo_defaultValues()
{
    UPowerHelper::BatteryInfo info;
    QCOMPARE(info.percentage, 0);
    QCOMPARE(info.state, 0u);
    QCOMPARE(info.isPresent, false);
}

void TestUPowerHelper::testBatteryInfo_isCharging()
{
    UPowerHelper::BatteryInfo info;

    info.state = 1;  // Charging
    QVERIFY(info.isCharging());

    info.state = 5;  // Pending charge
    QVERIFY(info.isCharging());

    info.state = 2;  // Discharging
    QVERIFY(!info.isCharging());
}

void TestUPowerHelper::testBatteryInfo_isFullyCharged()
{
    UPowerHelper::BatteryInfo info;

    info.state = 4;  // Fully charged
    QVERIFY(info.isFullyCharged());

    info.state = 1;  // Charging
    QVERIFY(!info.isFullyCharged());
}

void TestUPowerHelper::testBatteryInfo_isDischarging()
{
    UPowerHelper::BatteryInfo info;

    info.state = 2;  // Discharging
    QVERIFY(info.isDischarging());

    info.state = 1;  // Charging
    QVERIFY(!info.isDischarging());
}

void TestUPowerHelper::testPowerConfig_defaultValues()
{
    UPowerHelper::PowerConfig config;
    QCOMPARE(config.percentageLow, 20);
    QCOMPARE(config.percentageCritical, 5);
    QCOMPARE(config.percentageAction, 2);
    QCOMPARE(config.criticalAction, QString("HybridSleep"));
    QCOMPARE(config.handleLidSwitch, QString("suspend"));
    QCOMPARE(config.handleLidSwitchExternalPower, QString("suspend"));
}

void TestUPowerHelper::testReadPowerConfig_returnsValidStruct()
{
    // This test reads actual system config if available
    UPowerHelper::PowerConfig config = UPowerHelper::readPowerConfig();

    // Values should be within reasonable ranges
    QVERIFY(config.percentageLow >= 0 && config.percentageLow <= 100);
    QVERIFY(config.percentageCritical >= 0 && config.percentageCritical <= 100);
    QVERIFY(config.percentageAction >= 0 && config.percentageAction <= 100);
    QVERIFY(!config.criticalAction.isEmpty());
}

void TestUPowerHelper::testGetAvailablePowerProfiles_notEmpty()
{
    QStringList profiles = UPowerHelper::getAvailablePowerProfiles();

    // Should have at least the fallback profiles
    QVERIFY(!profiles.isEmpty());
    QVERIFY(profiles.size() >= 1);
}

QTEST_MAIN(TestUPowerHelper)
#include "test_upowerhelper.moc"
