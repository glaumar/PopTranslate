#include "appmain.h"

#include <KWindowSystem/kwindowsystem.h>
#include <kglobalaccel.h>
#include <ksystemclipboard.h>

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QDebug>
#include <QKeySequence>
#include <QMessageBox>
#include <QStandardPaths>
#include <QSystemTrayIcon>
#include <QTranslator>

#include "dictionaries.h"
#include "onlinetranslator.h"
#include "poptranslate.h"
#include "poptranslate_dbus.h"
#include "poptranslatesettings.h"

AppMain::AppMain(QObject *parent) : QObject(parent) {
    initClipboard();
    initTranslatorManager();
    initGlobalShortcuts();
    initSystemTrayIcon();
    initDBusInterface();
    initOcr();
    initTts();
    initPopupWindow();
}

void AppMain::initGlobalShortcuts() {
    translate_selection_act_ = new QAction(tr("Translate selection"), this);
    translate_selection_act_->setObjectName(APPLICATION_ID
                                            ".translate_selection");
    setShortcut(translate_selection_act_,
                PopTranslateSettings::instance().TranslateSelectionShortcut());
    connect(translate_selection_act_, &QAction::triggered, [this](bool unuse) {
        Q_UNUSED(unuse);
        translateSelection();
    });
    connect(&PopTranslateSettings::instance(),
            &PopTranslateSettings::translateSelectionShortcutChanged,
            this,
            [this](const QKeySequence &seq) {
                this->setShortcut(translate_selection_act_, seq);
            });

    ocr_act_ = new QAction(tr("Translate selected area"), this);
    ocr_act_->setObjectName(APPLICATION_ID ".ocr");
    setShortcut(ocr_act_, PopTranslateSettings::instance().OcrShortcut());
    connect(ocr_act_, &QAction::triggered, [this](bool unuse) {
        Q_UNUSED(unuse);
        this->grabber_.grabFullScreen();
    });
    connect(
        &PopTranslateSettings::instance(),
        &PopTranslateSettings::ocrShortcutChanged,
        this,
        [this](const QKeySequence &seq) { this->setShortcut(ocr_act_, seq); });
}

void AppMain::translateSelection() {
    translate(KSystemClipboard::instance()->text(QClipboard::Selection));
}

void AppMain::translate(const QString &text) {
    pop_.clear();
    pop_.setSourceText(text);
    if (text.isEmpty()) {
        return;
    }
    translator_manager_.translate(text);
    if (PopTranslateSettings::instance().isEnableAutoSpeak()) {
        tts_.speak(text);
    }
    pop_.show();
}

void AppMain::initSystemTrayIcon() {
    tray_.setToolTip(tr(APPLICATION_NAME));
    tray_.setIcon(QIcon::fromTheme(APPLICATION_ICON_BASE_NAME));

    // set systemtray menu
    QMenu *menu = new QMenu(&setting_window_);

    menu->addAction(
        QIcon::fromTheme("preferences-system-windows-effect-screenshot"),
        tr("Select Screen Area"),
        this,
        [this]() { grabber_.grabFullScreen(); });

    menu->addAction(QIcon::fromTheme("settings-configure"),
                    tr("Settings"),
                    this,
                    [this]() { setting_window_.show(); });

    menu->addAction(QIcon::fromTheme("application-exit"),
                    tr("Exit"),
                    this,
                    [this]() {
                        tray_.setVisible(false);
                        QApplication::quit();
                    });
    tray_.setContextMenu(menu);

    connect(&tray_, &QSystemTrayIcon::activated, this, &AppMain::trayActivated);
    tray_.show();
}

void AppMain::initDBusInterface() {
    PopTranslateDBus::instance()->registerService();
    connect(PopTranslateDBus::instance(),
            &PopTranslateDBus::receivedTranslateSelection,
            this,
            &AppMain::translateSelection);

    connect(PopTranslateDBus::instance(),
            &PopTranslateDBus::receivedTranslate,
            this,
            &AppMain::translate);
}

void AppMain::initClipboard() {
    // auto translate when clipboard changed
    connect(KSystemClipboard::instance(),
            &KSystemClipboard::changed,
            [this](QClipboard::Mode mode) {
                if (mode == QClipboard::Selection && pop_.isVisible() &&
                    PopTranslateSettings::instance().monitorClipboard()) {
                    translateSelection();
                }
            });
}

void AppMain::initOcr() {
    connect(&grabber_,
            &ScreenGrabber::screenshotReady,
            &cropper_,
            &ImageCropper::cropImage);
    connect(&cropper_, &ImageCropper::imageCropped, [this](QPixmap img) {
        ocr_.recognize(img);
    });
    connect(&ocr_,
            &Ocr::recognized,
            this,
            &AppMain::translate,
            Qt::QueuedConnection);
    connect(&PopTranslateSettings::instance(),
            &PopTranslateSettings::ocrLanguagesChanged,
            &ocr_,
            [this](const QStringList &languages) {
                this->ocr_.init(languages.join("+").toLocal8Bit(), "", {});
            });

    ocr_.init(
        PopTranslateSettings::instance().ocrLanguages().join("+").toLocal8Bit(),
        "",
        {});
}

void AppMain::initTts() {
    connect(&pop_, &PopupDialog::requestSpeak, &tts_, &Tts::speak);
}

void AppMain::initTranslatorManager() {
    translator_manager_.addTranslator(
        new OnlineTranslator(&translator_manager_));
    translator_manager_.addTranslator(new Dictionaries(&translator_manager_));
    connect(&translator_manager_,
            &TranslatorManager::resultAvailable,
            &pop_,
            &PopupDialog::addTranslateResult);

    connect(&pop_, &PopupDialog::requestTranslate, this, &AppMain::translate);
}

void AppMain::initPopupWindow() {
    // Show setting window when PopupDialog contextmenu action "settings"
    // triggered
    connect(&pop_, &PopupDialog::requestShowSettingsWindow, [this] {
        setting_window_.show();
    });

    // stop translate and speking when PopupDialog hidden
    connect(&pop_, &PopupDialog::hidden, [this] {
        tts_.stop();
        translator_manager_.abortAll();
    });
}

void AppMain::trayActivated(QSystemTrayIcon::ActivationReason reason) {
    Q_UNUSED(reason);
    bool is_visible = setting_window_.isVisible();
    setting_window_.setVisible(!is_visible);
}

bool AppMain::setShortcut(QAction *act, const QKeySequence &seq) {
    KGlobalAccel::self()->removeAllShortcuts(act);
    bool ret = KGlobalAccel::setGlobalShortcut(act, seq);

    if (seq.isEmpty()) {
        qDebug() << tr("Unbind global shortcuts");
    } else {
        qDebug() << tr("Bind global shortcuts (%1) %2")
                        .arg(seq.toString(),
                             ret ? tr("success") : tr("failed"));
    }
    return ret;
}