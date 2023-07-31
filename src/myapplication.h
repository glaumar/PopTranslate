#pragma once

#include <ksystemclipboard.h>

#include <QApplication>
#include <QSystemTrayIcon>

#include "imagecropper.h"
#include "popupdialog.h"
#include "screengrabber.h"
#include "settingwindow.h"

class MyApplication : public QApplication {
    Q_OBJECT

   public:
    MyApplication(int& argc, char** argv);
    ~MyApplication();

   private slots:
    void showPop();
    void trayActivated(QSystemTrayIcon::ActivationReason reason);
    bool setShortcut(const QKeySequence& seq);

   private:
    void initUiTranslator();
    void initGlobalShortcuts();
    void initSystemTrayIcon();
    void initDBusInterface();
    void loadSettings();
    void loadDictionaries();
    void initClipboard();
    void initOcr();

    QTranslator* translator_;  // init in initUiTranslator
    PopupDialog* pop_;
    QSystemTrayIcon* tray_;
    SettingWindow* setting_window_;
    KSystemClipboard* clipboard_;  // init in initClipboard
    QAction* shortcut_act_;        // init in initGlobalShortcuts
    ScreenGrabber *grabber_;  
    ImageCropper *cropper_;
};
