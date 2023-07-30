#pragma once

#include <QDBusInterface>
#include <QObject>
#include <QPixmap>

class ScreenGrabber : public QObject {
    Q_OBJECT
   public:
    explicit ScreenGrabber(QObject *parent = nullptr);
    void grabFullScreen();

   private slots:
    void onResponse(uint32_t response, QVariantMap results);

   signals:
    void screenshotReady(QPixmap pixmap);

   private:
    QDBusInterface screenshot_interface_;
};