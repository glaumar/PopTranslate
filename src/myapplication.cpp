#include "myapplication.h"

#include <KWindowSystem/kwindowsystem.h>
#include <kglobalaccel.h>

#include <QAction>
#include <QClipboard>
#include <QCloseEvent>
#include <QDebug>
#include <QKeySequence>
#include <QMessageBox>
#include <QStandardPaths>
#include <QSystemTrayIcon>
#include <QTranslator>

#include "poptranslate_dbus.h"

MyApplication::MyApplication(int &argc, char **argv)
    : QApplication(argc, argv) {
    setApplicationName(QStringLiteral("PopTranslate"));
    setDesktopFileName(
        QStringLiteral("io.github.glaumar.PopTranslate.desktop"));

    // Must install translator before create any ui
    initUiTranslator();

    pop_ = new PopupDialog(nullptr);
    tray_ = new QSystemTrayIcon(nullptr);
    setting_window_ = new SettingWindow(nullptr);

    initClipboard();
    initGlobalShortcuts();
    initSystemTrayIcon();
    initDBusInterface();
    loadSettings();
    loadDictionaries();
    initOcr();
}

MyApplication::~MyApplication() {
    // save settings
    setting_window_->setPopupWindowSize(pop_->size());
    setting_window_->setShowSrcText(pop_->isSrcTextEditVisible());

    delete setting_window_;
    delete tray_;
    delete pop_;
    delete translator_;
    delete grabber_;
    delete cropper_;
}

void MyApplication::initUiTranslator() {
    translator_ = new QTranslator(nullptr);
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString base_name = "poptranslate_" + QLocale(locale).name();
        QString i18n_dir =
            QStandardPaths::locate(QStandardPaths::AppDataLocation,
                                   "i18n",
                                   QStandardPaths::LocateDirectory);
        if (translator_->load(base_name, i18n_dir, "_", ".qm")) {
            bool ret = this->installTranslator(translator_);
            qDebug() << tr("Loaded %1.qm %2")
                            .arg(base_name, ret ? tr("success") : tr("failed"));
            break;
        }
    }
}

void MyApplication::initGlobalShortcuts() {
    auto obj_name_base = QString("io.github.glaumar.PopTranslate.%1");

    translate_selection_act_ = new QAction(tr("Translate selection"), this);
    translate_selection_act_->setObjectName(
        obj_name_base.arg("translate_selection"));
    setShortcut(translate_selection_act_,
                setting_window_->TranslateSelectionShortcut());
    connect(translate_selection_act_, &QAction::triggered, [this](bool unuse) {
        Q_UNUSED(unuse);
        this->showPop();
    });
    connect(setting_window_,
            &SettingWindow::TranslateSelectionShortcutChanged,
            this,
            [this](const QKeySequence &seq) {
                this->setShortcut(translate_selection_act_, seq);
            });

    ocr_act_ = new QAction(tr("Translate selected area"), this);
    ocr_act_->setObjectName(obj_name_base.arg("ocr"));
    setShortcut(ocr_act_, setting_window_->OcrShortcut());
    connect(ocr_act_, &QAction::triggered, [this](bool unuse) {
        Q_UNUSED(unuse);
        this->grabber_->grabFullScreen();
    });
    connect(
        setting_window_,
        &SettingWindow::OcrShortcutChanged,
        this,
        [this](const QKeySequence &seq) { this->setShortcut(ocr_act_, seq); });
}

void MyApplication::showPop() {
    if (pop_->isVisible() && !pop_->isNormalWindow()) {
        pop_->hide();
        return;
    }
    pop_->translate(clipboard_->text(QClipboard::Selection));
    pop_->show();
}

void MyApplication::initSystemTrayIcon() {
    tray_->setToolTip(tr("PopTranslate"));
    tray_->setIcon(
        QIcon::fromTheme(QStringLiteral("io.github.glaumar.PopTranslate")));

    // set systemtray menu
    QMenu *menu = new QMenu(setting_window_);

    menu->addAction(
        QIcon::fromTheme("preferences-system-windows-effect-screenshot"),
        tr("Select screen area"),
        this,
        [this]() { grabber_->grabFullScreen(); });

    menu->addAction(QIcon::fromTheme("settings-configure"),
                    tr("Settings"),
                    this,
                    [this]() { setting_window_->show(); });

    menu->addAction(QIcon::fromTheme("application-exit"),
                    tr("Exit"),
                    this,
                    [this]() {
                        tray_->setVisible(false);
                        this->quit();
                    });
    tray_->setContextMenu(menu);

    connect(tray_,
            &QSystemTrayIcon::activated,
            this,
            &MyApplication::trayActivated);
    tray_->show();
}

void MyApplication::initDBusInterface() {
    PopTranslateDBus::instance()->registerService();
    connect(PopTranslateDBus::instance(),
            &PopTranslateDBus::receivedTranslateSelection,
            this,
            &MyApplication::showPop);

    connect(PopTranslateDBus::instance(),
            &PopTranslateDBus::receivedTranslate,
            this,
            [this](const QString &text) {
                this->pop_->translate(text);
                this->pop_->show();
            });
}

void MyApplication::loadSettings() {
    // Change settings for popupdialog when settings changed
    connect(setting_window_,
            &SettingWindow::translateEngineChanged,
            pop_,
            &PopupDialog::setTranslateEngine);

    connect(setting_window_,
            &SettingWindow::targetLanguagesChanged,
            pop_,
            &PopupDialog::setTargetLanguages);

    connect(setting_window_,
            &SettingWindow::fontChanged,
            pop_,
            &PopupDialog::setFont);

    connect(setting_window_,
            &SettingWindow::opacityChanged,
            pop_,
            &PopupDialog::setOpacity);

    connect(setting_window_,
            &SettingWindow::triggerBlurEffect,
            pop_,
            &PopupDialog::enableBlur);

    // load settings for popupdialog
    pop_->setTranslateEngine(setting_window_->translateEngine());
    pop_->setTargetLanguages(setting_window_->targetLanguages());
    pop_->setFont(setting_window_->font());
    pop_->setOpacity(setting_window_->opacity());
    pop_->enableBlur(setting_window_->isEnableBlur());

    // Show setting window when PopupDialog contextmenu action "settings"
    // triggered
    connect(pop_, &PopupDialog::settingsActionTriggered, [this]() {
        setting_window_->show();
    });

    // resize popuo window size
    if (!setting_window_->popupWindowSize().isEmpty()) {
        pop_->resize(setting_window_->popupWindowSize());
    }

    pop_->setSrcTextEditVisible(setting_window_->showSrcText());
}

void MyApplication::loadDictionaries() {
    connect(setting_window_,
            &SettingWindow::dictionariesChanged,
            pop_,
            &PopupDialog::setDictionaries);

    // connect(setting_window_,
    //         &SettingWindow::dictionaryRemoved,
    //         [this](const QString &dict) {
    //             this->pop_->removeDictionaries({dict});
    //         });

    pop_->setDictionaries(setting_window_->dictionaries());
}

void MyApplication::initClipboard() {
    clipboard_ = KSystemClipboard::instance();
    // auto translate when clipboard changed
    connect(clipboard_,
            &KSystemClipboard::changed,
            [this](QClipboard::Mode mode) {
                // only translate when popupdialog is visible
                if (mode == QClipboard::Selection && pop_->isVisible()) {
                    pop_->translate(clipboard_->text(QClipboard::Selection));
                }
            });
}

void MyApplication::initOcr() {
    grabber_ = new ScreenGrabber(nullptr);
    cropper_ = new ImageCropper(nullptr);

    connect(grabber_,
            &ScreenGrabber::screenshotReady,
            cropper_,
            &ImageCropper::cropImage);
    connect(cropper_, &ImageCropper::imageCropped, [this](QPixmap img) {
        ocr_.recognize(img);
    });
    connect(
        &ocr_,
        &Ocr::recognized,
        this,
        [this](const QString &text) {
            qDebug() << text;
            this->pop_->show();
            this->pop_->translate(text);
        },
        Qt::QueuedConnection);
    connect(setting_window_,
            &SettingWindow::ocrLanguagesChanged,
            &ocr_,
            [this](const QStringList &languages) {
                this->ocr_.init(languages.join("+").toLocal8Bit(), "", {});
            });

    ocr_.init("eng", "", {});
    setting_window_->setAvailableOcrLanguages(ocr_.availableLanguages());
}

void MyApplication::trayActivated(QSystemTrayIcon::ActivationReason reason) {
    Q_UNUSED(reason);
    bool is_visible = setting_window_->isVisible();
    setting_window_->setVisible(!is_visible);
}

bool MyApplication::setShortcut(QAction *act, const QKeySequence &seq) {
    KGlobalAccel::self()->removeAllShortcuts(act);
    bool ret = KGlobalAccel::setGlobalShortcut(act, seq);
    // bool ret = KGlobalAccel::self()->setGlobalShortcut(act, {seq});

    if (seq.isEmpty()) {
        qDebug() << tr("Unbind global shortcuts");
    } else {
        qDebug() << tr("Bind global shortcuts (%1) %2")
                        .arg(seq.toString(),
                             ret ? tr("success") : tr("failed"));
    }
    return ret;
}
