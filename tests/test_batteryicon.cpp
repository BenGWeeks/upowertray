#include <QtTest/QtTest>
#include "../src/batteryicon.h"

class TestBatteryIcon : public QObject
{
    Q_OBJECT

private slots:
    void testGetColor_green();
    void testGetColor_amber();
    void testGetColor_red();
    void testGetOutlineColor_white();
    void testGetOutlineColor_amber();
    void testGetOutlineColor_red();
    void testCreate_validIcon();
    void testCreate_differentSizes();
    void testCreate_charging();
};

void TestBatteryIcon::testGetColor_green()
{
    BatteryIcon::Thresholds thresholds{20, 5};
    QColor color = BatteryIcon::getColor(50, thresholds);
    QCOMPARE(color, QColor(50, 200, 50));
}

void TestBatteryIcon::testGetColor_amber()
{
    BatteryIcon::Thresholds thresholds{20, 5};
    QColor color = BatteryIcon::getColor(15, thresholds);
    QCOMPARE(color, QColor(220, 180, 50));
}

void TestBatteryIcon::testGetColor_red()
{
    BatteryIcon::Thresholds thresholds{20, 5};
    QColor color = BatteryIcon::getColor(3, thresholds);
    QCOMPARE(color, QColor(220, 50, 50));
}

void TestBatteryIcon::testGetOutlineColor_white()
{
    BatteryIcon::Thresholds thresholds{20, 5};
    QColor color = BatteryIcon::getOutlineColor(50, thresholds);
    QCOMPARE(color, QColor(Qt::white));
}

void TestBatteryIcon::testGetOutlineColor_amber()
{
    BatteryIcon::Thresholds thresholds{20, 5};
    QColor color = BatteryIcon::getOutlineColor(15, thresholds);
    QCOMPARE(color, QColor(220, 180, 50));
}

void TestBatteryIcon::testGetOutlineColor_red()
{
    BatteryIcon::Thresholds thresholds{20, 5};
    QColor color = BatteryIcon::getOutlineColor(3, thresholds);
    QCOMPARE(color, QColor(220, 50, 50));
}

void TestBatteryIcon::testCreate_validIcon()
{
    BatteryIcon::Thresholds thresholds{20, 5};
    QIcon icon = BatteryIcon::create(22, 50, false, thresholds);
    QVERIFY(!icon.isNull());
}

void TestBatteryIcon::testCreate_differentSizes()
{
    BatteryIcon::Thresholds thresholds{20, 5};

    QIcon small = BatteryIcon::create(22, 50, false, thresholds);
    QIcon large = BatteryIcon::create(64, 50, false, thresholds);

    QVERIFY(!small.isNull());
    QVERIFY(!large.isNull());

    // Icons should have requested sizes available
    QVERIFY(small.availableSizes().contains(QSize(22, 22)));
    QVERIFY(large.availableSizes().contains(QSize(64, 64)));
}

void TestBatteryIcon::testCreate_charging()
{
    BatteryIcon::Thresholds thresholds{20, 5};

    QIcon notCharging = BatteryIcon::create(22, 50, false, thresholds);
    QIcon charging = BatteryIcon::create(22, 50, true, thresholds);

    QVERIFY(!notCharging.isNull());
    QVERIFY(!charging.isNull());

    // Both should be valid icons (visual difference tested manually)
}

QTEST_MAIN(TestBatteryIcon)
#include "test_batteryicon.moc"
