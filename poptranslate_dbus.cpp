#include "poptranslate_dbus.h"
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDebug>

PopTranslateDBus::PopTranslateDBus(QObject *parent) : QObject(parent) {
    QDBusConnection::sessionBus().registerService("io.github.glaumar.PopTranslate");
    QDBusConnection::sessionBus().registerObject("/PopTranslate", this, QDBusConnection::ExportAllSlots);
}

PopTranslateDBus *PopTranslateDBus::instance() {
    static PopTranslateDBus instance;
    return &instance;
}

void PopTranslateDBus::callTranslateSelection() {
    QDBusMessage msg = QDBusMessage::createMethodCall("io.github.glaumar.PopTranslate",
                                                      "/PopTranslate",
                                                      "io.github.glaumar.PopTranslate",
                                                      "translateSelection");
    QDBusConnection::sessionBus().call(msg);
}

// Service is registered in constructor
void PopTranslateDBus::registerService() {}

bool PopTranslateDBus::isRegistered() {
    return QDBusConnection::sessionBus().interface()->isServiceRegistered("io.github.glaumar.PopTranslate");
}

void PopTranslateDBus::translateSelection() {
    emit receivedTranslateSelection();
}

void PopTranslateDBus::translate(const QString& text) {
    emit receivedTranslate(text);
}