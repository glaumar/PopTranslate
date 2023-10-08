#include <QApplication>
#include <QLocale>
#include <QStandardPaths>
#include <QTranslator>

#include "appmain.h"
#include "poptranslate.h"
#include "poptranslate_dbus.h"
#include "pybind11_wrap.h"

int main(int argc, char *argv[]) {
    if (PopTranslateDBus::isRegistered()) {
        // PopTranslate is already running, using dbus to call
        // translateSelection
        PopTranslateDBus::callTranslateSelection();
        return 0;
    } else {
        // register PopTranslate dbus service
        PopTranslateDBus::instance()->registerService();
    }

    QApplication a(argc, argv);
    a.setApplicationName(PROJECT_NAME);
    a.setDesktopFileName(DESKTOP_FILE_NAME);
    a.setApplicationVersion(APPLICATION_VERSION);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString base_name = "poptranslate_" + QLocale(locale).name();
        QString i18n_dir =
            QStandardPaths::locate(QStandardPaths::AppDataLocation,
                                   "i18n",
                                   QStandardPaths::LocateDirectory);
        if (translator.load(base_name, i18n_dir, "_", ".qm")) {
            bool ret = a.installTranslator(&translator);
            break;
        }
    }

    // initialize python interpreter
    pybind11::scoped_interpreter python;
    // release GIL
    pybind11::gil_scoped_release release;

    AppMain app_main;

    return a.exec();
}