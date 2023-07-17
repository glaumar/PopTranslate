#include <QLocale>

#include "myapplication.h"
#include "poptranslate_dbus.h"

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

    MyApplication a(argc, argv);

    return a.exec();
}
