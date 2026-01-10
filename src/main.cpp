#include "batterytray.h"

#include <QApplication>
#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("upowertray");
    app.setApplicationVersion("1.1.0");
    app.setOrganizationName("upowertray");
    app.setQuitOnLastWindowClosed(false);

    // Load translations
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "upowertray_" + QLocale(locale).name().section('_', 0, 0);
        if (translator.load(baseName, "/usr/share/upowertray/translations") ||
            translator.load(baseName, ":/translations") ||
            translator.load(baseName, QApplication::applicationDirPath() + "/translations")) {
            app.installTranslator(&translator);
            break;
        }
    }

    BatteryTray batteryTray;

    return app.exec();
}
