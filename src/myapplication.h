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
    MyApplication(int& argc, char** argv);
    ~MyApplication();

   private slots:
    void showPop(bool unuse);
    void trayActivated(QSystemTrayIcon::ActivationReason reason);
    bool setShortcut(const QList<QKeySequence>& shortcuts);

   private:
    void initUiTranslator();
    void initGlobalShortcuts();
    void initSystemTrayIcon();
    void initDBusInterface();
    void loadSettings();

    QTranslator *translator_;
    PopupDialog* pop_;
    QSystemTrayIcon* tray_;
    SettingWindow* setting_window_;
    KSystemClipboard* clipboard_;
    QAction* shortcut_act_;
};
#endif  // MYAPPLICATION_H
