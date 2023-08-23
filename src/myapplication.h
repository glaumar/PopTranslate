#pragma once

#include <ksystemclipboard.h>

#include <QApplication>
#include <QSystemTrayIcon>

#include "imagecropper.h"
#include "ocr.h"
#include "popupdialog.h"
#include "screengrabber.h"
#include "settingwindow.h"
#include "translatormanager.h"

class MyApplication : public QApplication {
    Q_OBJECT

   public:
    MyApplication(int& argc, char** argv);
    // ~MyApplication();

    //    private slots:
    //     // void showPop();
    //     void translate(const QString& text);
    //     void translateSelection();
    //     void trayActivated(QSystemTrayIcon::ActivationReason reason);
    //     bool setShortcut(QAction* act, const QKeySequence& seq);

   private:
    void initUiTranslator();
    // void initGlobalShortcuts();
    // void initSystemTrayIcon();
    // void initDBusInterface();
    // void initClipboard();
    // void initOcr();
    // void initTranslatorManager();

    QTranslator* translator_;
    // PopupDialog* pop_;
    // QSystemTrayIcon* tray_;
    // SettingWindow* setting_window_;
    // QAction* translate_selection_act_;
    // QAction* ocr_act_;
    // ScreenGrabber* grabber_;
    // ImageCropper* cropper_;
    // Ocr ocr_;
    // TranslatorManager translator_manager_;
};
