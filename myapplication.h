#ifndef MYAPPLICATION_H
#define MYAPPLICATION_H

#include <ksystemclipboard.h>

#include <QApplication>
#include <QSystemTrayIcon>

#include "popupdialog.h"
#include "settingwindow.h"

class MyApplication : public QApplication {
    Q_OBJECT

   public:
    MyApplication(int &argc, char **argv);

   private slots:
    void showPop(bool unuse);
    void trayActivated(QSystemTrayIcon::ActivationReason reason);

   private:
    bool initGlobalShortcuts();
    void initSystemTrayIcon();
    void initDBusInterface();
    void loadSettings();

    PopupDialog pop_;
    QSystemTrayIcon tray_;
    SettingWindow setting_window_;
    KSystemClipboard *clipboard_;
};
#endif  // MYAPPLICATION_H
