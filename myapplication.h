#ifndef MYAPPLICATION_H
#define MYAPPLICATION_H

#include <ksystemclipboard.h>

#include <QApplication>
#include <QSystemTrayIcon>

#include "popupdialog.h"

// QT_BEGIN_NAMESPACE
// namespace Ui { class MyApplication; }
// QT_END_NAMESPACE

class MyApplication : public QApplication {
    Q_OBJECT

   public:
    // explicit MyApplication(QWidget *parent = nullptr);
    MyApplication(int &argc, char **argv);

   private slots:
    void showPop(bool unuse);
    void trayActivated(QSystemTrayIcon::ActivationReason reason);

   private:
    bool initGlobalShortcuts();
    void initSystemTrayIcon();
    void initDBusInterface();

    PopupDialog pop;
    QSystemTrayIcon tray;
    KSystemClipboard *clipboard;
};
#endif  // MYAPPLICATION_H
