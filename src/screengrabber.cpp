#include "screengrabber.h"

#include <QDBusInterface>
#include <QDBusPendingReply>
#include <QDebug>

ScreenGrabber::ScreenGrabber(QObject *parent)
    : QObject(parent),
      screenshot_interface_(
          QStringLiteral("org.freedesktop.portal.Desktop"),
          QStringLiteral("/org/freedesktop/portal/desktop"),
          QStringLiteral("org.freedesktop.portal.Screenshot")) {}

void ScreenGrabber::grabFullScreen() {
    // https://flatpak.github.io/xdg-desktop-portal/#gdbus-org.freedesktop.portal.Screenshot

    QVariantMap options;
    options["handle_token"] = "pop_screenshot";
    // options["modal"] = false;
    // options["interactive"] = true;
    QDBusMessage reply =
        screenshot_interface_.call(QStringLiteral("Screenshot"), "", options);

    if (reply.type() == QDBusMessage::ErrorMessage) {
        qCritical() << tr("Error: %1").arg(reply.errorMessage());
    } else {
        QString path = reply.arguments().at(0).value<QDBusObjectPath>().path();

        QDBusConnection::sessionBus().connect(
            QStringLiteral("org.freedesktop.portal.Desktop"),
            path,
            QStringLiteral("org.freedesktop.portal.Request"),
            QStringLiteral("Response"),
            QStringLiteral("ua{sv}"),
            this,
            SLOT(onResponse(uint32_t, QVariantMap)));
    }
}

void ScreenGrabber::onResponse(uint32_t response, QVariantMap results) {
    if (!response) {
        QString filename = results["uri"].value<QString>();
        emit screenshotReady(QPixmap(filename));
    } else {
        qCritical() << tr("Error: take screenshot failed");
    }
}