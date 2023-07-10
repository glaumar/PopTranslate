
#include <QLocale>
#include <QTranslator>

#include "myapplication.h"
#include "poptranslate_dbus.h"

int main(int argc, char *argv[]) {
    if (PopTranslateDBus::isRegistered()) {
        PopTranslateDBus::callTranslateSelection();
        return 0;
    } else {
        PopTranslateDBus::instance()->registerService();
    }

    MyApplication a(argc, argv);
    a.setDesktopFileName("io.github.glaumar.PopTranslate.desktop");

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "poptranslate_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    // MainWindow w;
    // w.setWindowTitle("PopTranslate");
    // w.show();
    return a.exec();
}
