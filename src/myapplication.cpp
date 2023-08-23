#include "myapplication.h"

// #include <KWindowSystem/kwindowsystem.h>
// #include <kglobalaccel.h>

// #include <QAction>
// #include <QClipboard>
// #include <QCloseEvent>
// #include <QDebug>
// #include <QKeySequence>
// #include <QMessageBox>
#include <QStandardPaths>
// #include <QSystemTrayIcon>
#include <QTranslator>

// #include "dictionaries.h"
// #include "onlinetranslator.h"
// #include "poptranslate.h"
// #include "poptranslate_dbus.h"
// #include "poptranslatesettings.h"

MyApplication::MyApplication(int &argc, char **argv)
    : QApplication(argc, argv) {
    setApplicationName(PROJECT_NAME);
    setDesktopFileName(DESKTOP_FILE_NAME);
    setApplicationVersion(APPLICATION_VERSION);

    // Must install translator before create any ui
    initUiTranslator();

    // pop_ = new PopupDialog(nullptr);
    // tray_ = new QSystemTrayIcon(nullptr);
    // setting_window_ = new SettingWindow(nullptr);

    // initClipboard();
    // initGlobalShortcuts();
    // initSystemTrayIcon();
    // initDBusInterface();
    // initOcr();
    // initTranslatorManager();

    // // Show setting window when PopupDialog contextmenu action "settings"
    // // triggered
    // connect(pop_, &PopupDialog::settingsActionTriggered, [this]() {
    //     setting_window_->show();
    // });
}

// MyApplication::~MyApplication() {
//     delete setting_window_;
//     delete tray_;
//     delete pop_;
//     delete translator_;
//     delete grabber_;
//     delete cropper_;
// }

void MyApplication::initUiTranslator() {
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

// void MyApplication::initGlobalShortcuts() {
//     translate_selection_act_ = new QAction(tr("Translate selection"), this);
//     translate_selection_act_->setObjectName(APPLICATION_ID
//                                             ".translate_selection");
//     setShortcut(translate_selection_act_,
//                 PopTranslateSettings::instance().TranslateSelectionShortcut());
//     connect(translate_selection_act_, &QAction::triggered, [this](bool unuse)
//     {
//         Q_UNUSED(unuse);
//         translateSelection();
//     });
//     connect(&PopTranslateSettings::instance(),
//             &PopTranslateSettings::translateSelectionShortcutChanged,
//             this,
//             [this](const QKeySequence &seq) {
//                 this->setShortcut(translate_selection_act_, seq);
//             });

//     ocr_act_ = new QAction(tr("Translate selected area"), this);
//     ocr_act_->setObjectName(APPLICATION_ID ".ocr");
//     setShortcut(ocr_act_, PopTranslateSettings::instance().OcrShortcut());
//     connect(ocr_act_, &QAction::triggered, [this](bool unuse) {
//         Q_UNUSED(unuse);
//         this->grabber_->grabFullScreen();
//     });
//     connect(
//         &PopTranslateSettings::instance(),
//         &PopTranslateSettings::ocrShortcutChanged,
//         this,
//         [this](const QKeySequence &seq) { this->setShortcut(ocr_act_, seq);
//         });
// }

// void MyApplication::translateSelection() {
//     translate(KSystemClipboard::instance()->text(QClipboard::Selection));
// }

// void MyApplication::translate(const QString &text) {
//     // if (pop_->isVisible() && !pop_->isNormalWindow()) {
//     //     pop_->hide();
//     //     return;
//     // }
//     pop_->clear();
//     pop_->setSourceText(text);
//     translator_manager_.translate(text);
//     pop_->show();
// }

// void MyApplication::initSystemTrayIcon() {
//     tray_->setToolTip(tr(APPLICATION_NAME));
//     tray_->setIcon(QIcon::fromTheme(APPLICATION_ICON_BASE_NAME));

//     // set systemtray menu
//     QMenu *menu = new QMenu(setting_window_);

//     menu->addAction(
//         QIcon::fromTheme("preferences-system-windows-effect-screenshot"),
//         tr("Select Screen Area"),
//         this,
//         [this]() { grabber_->grabFullScreen(); });

//     menu->addAction(QIcon::fromTheme("settings-configure"),
//                     tr("Settings"),
//                     this,
//                     [this]() { setting_window_->show(); });

//     menu->addAction(QIcon::fromTheme("application-exit"),
//                     tr("Exit"),
//                     this,
//                     [this]() {
//                         tray_->setVisible(false);
//                         this->quit();
//                     });
//     tray_->setContextMenu(menu);

//     connect(tray_,
//             &QSystemTrayIcon::activated,
//             this,
//             &MyApplication::trayActivated);
//     tray_->show();
// }

// void MyApplication::initDBusInterface() {
//     PopTranslateDBus::instance()->registerService();
//     connect(PopTranslateDBus::instance(),
//             &PopTranslateDBus::receivedTranslateSelection,
//             this,
//             &MyApplication::translateSelection);

//     connect(PopTranslateDBus::instance(),
//             &PopTranslateDBus::receivedTranslate,
//             this,
//             &MyApplication::translate);
// }

// void MyApplication::initClipboard() {
//     // auto translate when clipboard changed
//     connect(KSystemClipboard::instance(),
//             &KSystemClipboard::changed,
//             [this](QClipboard::Mode mode) {
//                 // only translate when popupdialog is visible
//                 if (mode == QClipboard::Selection && pop_->isVisible()) {
//                     translateSelection();
//                 }
//             });
// }

// void MyApplication::initOcr() {
//     grabber_ = new ScreenGrabber(nullptr);
//     cropper_ = new ImageCropper(nullptr);

//     connect(grabber_,
//             &ScreenGrabber::screenshotReady,
//             cropper_,
//             &ImageCropper::cropImage);
//     connect(cropper_, &ImageCropper::imageCropped, [this](QPixmap img) {
//         ocr_.recognize(img);
//     });
//     connect(&ocr_,
//             &Ocr::recognized,
//             this,
//             &MyApplication::translate,
//             Qt::QueuedConnection);
//     connect(&PopTranslateSettings::instance(),
//             &PopTranslateSettings::ocrLanguagesChanged,
//             &ocr_,
//             [this](const QStringList &languages) {
//                 this->ocr_.init(languages.join("+").toLocal8Bit(), "", {});
//             });

//     ocr_.init(
//         PopTranslateSettings::instance().ocrLanguages().join("+").toLocal8Bit(),
//         "",
//         {});
// }

// void MyApplication::initTranslatorManager() {
//     translator_manager_.addTranslator(new
//     Dictionaries(&translator_manager_)); translator_manager_.addTranslator(
//         new OnlineTranslator(&translator_manager_));
//     connect(&translator_manager_,
//             &TranslatorManager::resultAvailable,
//             pop_,
//             &PopupDialog::addTranslateResult);

//     connect(pop_,
//             &PopupDialog::requestTranslate,
//             this,
//             &MyApplication::translate);
// }

// void MyApplication::trayActivated(QSystemTrayIcon::ActivationReason reason) {
//     Q_UNUSED(reason);
//     bool is_visible = setting_window_->isVisible();
//     setting_window_->setVisible(!is_visible);
// }

// bool MyApplication::setShortcut(QAction *act, const QKeySequence &seq) {
//     KGlobalAccel::self()->removeAllShortcuts(act);
//     bool ret = KGlobalAccel::setGlobalShortcut(act, seq);

//     if (seq.isEmpty()) {
//         qDebug() << tr("Unbind global shortcuts");
//     } else {
//         qDebug() << tr("Bind global shortcuts (%1) %2")
//                         .arg(seq.toString(),
//                              ret ? tr("success") : tr("failed"));
//     }
//     return ret;
// }
