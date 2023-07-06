#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include "popupdialog.h"
#include <ksystemclipboard.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    // TODO: add dbus method for popup
protected:
    virtual void closeEvent(QCloseEvent *event) override;

private slots:
    void showPop(bool unuse);
    void trayActivated(QSystemTrayIcon::ActivationReason reason);

private:
    bool initGlobalShortcuts();
    void initSystemTrayIcon();

    Ui::MainWindow *ui;
    PopupDialog *pop;
    QSystemTrayIcon *tray;
    KSystemClipboard *clipboard;
};
#endif // MAINWINDOW_H
