#include <QApplication>
#include "batterytray.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("upowertray");
    app.setApplicationVersion("1.0.0");
    app.setQuitOnLastWindowClosed(false);

    BatteryTray batteryTray;

    return app.exec();
}
