#pragma once
#include <QSystemTrayIcon>

#include "imagecropper.h"
#include "ocr.h"
#include "popupdialog.h"
#include "screengrabber.h"
#include "settingwindow.h"
#include "translatormanager.h"
#include "tts.h"

class AppMain : public QObject {
    Q_OBJECT

   public:
    explicit AppMain(QObject* parent = nullptr);

   private slots:
    void translate(const QString& text);
    void translateSelection();
    void trayActivated(QSystemTrayIcon::ActivationReason reason);
    bool setShortcut(QAction* act, const QKeySequence& seq);

   private:
    void initGlobalShortcuts();
    void initSystemTrayIcon();
    void initDBusInterface();
    void initClipboard();
    void initOcr();
    void initTts();
    void initTranslatorManager();

    PopupDialog pop_;
    QSystemTrayIcon tray_;
    SettingWindow setting_window_;
    QAction* translate_selection_act_;
    QAction* ocr_act_;
    ScreenGrabber grabber_;
    ImageCropper cropper_;
    Ocr ocr_;
    Tts tts_;
    TranslatorManager translator_manager_;
};