#pragma once

#include <QObject>
#include <QString>

class PopTranslateDBus : public QObject {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "io.github.glaumar.PopTranslate")

   public:
    static PopTranslateDBus* instance();
    static void callTranslateSelection();
    void registerService();
    static bool isRegistered();

    PopTranslateDBus(PopTranslateDBus const&) = delete;
    void operator=(PopTranslateDBus const&) = delete;
    PopTranslateDBus(PopTranslateDBus&&) = delete;
    void operator=(PopTranslateDBus&&) = delete;

   public slots:
    // dbus method
    void translateSelection();
    void translate(const QString& text);

   signals:
    void receivedTranslateSelection();
    void receivedTranslate(const QString& text);

   private:
    explicit PopTranslateDBus(QObject* parent = nullptr);
};