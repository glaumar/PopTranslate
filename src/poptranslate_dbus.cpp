#include "poptranslate_dbus.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDebug>

PopTranslateDBus::PopTranslateDBus(QObject *parent) : QObject(parent) {
    QDBusConnection::sessionBus().registerService(APPLICATION_ID);
    QDBusConnection::sessionBus().registerObject(
        "/" APPLICATION_NAME,
        this,
        QDBusConnection::ExportAllSlots);
}

PopTranslateDBus *PopTranslateDBus::instance() {
    static PopTranslateDBus instance;
    return &instance;
}

void PopTranslateDBus::callTranslateSelection() {
    QDBusMessage msg = QDBusMessage::createMethodCall(APPLICATION_ID,
                                                      "/" APPLICATION_NAME,
                                                      APPLICATION_ID,
                                                      "translateSelection");
    QDBusConnection::sessionBus().call(msg);
}

// Service is registered in constructor
void PopTranslateDBus::registerService() {}

bool PopTranslateDBus::isRegistered() {
    return QDBusConnection::sessionBus().interface()->isServiceRegistered(
        APPLICATION_ID);
}

void PopTranslateDBus::translateSelection() {
    emit receivedTranslateSelection();
}

void PopTranslateDBus::translate(const QString &text) {
    emit receivedTranslate(text);
}