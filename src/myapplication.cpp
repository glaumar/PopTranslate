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
    : QApplication(argc, argv), clipboard_(KSystemClipboard::instance()) {
    setApplicationName("PopTranslate");
    setDesktopFileName("io.github.glaumar.PopTranslate.desktop");
    // Must install translator before create any ui
    initUiTranslator();

    pop_ = new PopupDialog(nullptr);
    tray_ = new QSystemTrayIcon(nullptr);
    setting_window_ = new SettingWindow(nullptr);

    initGlobalShortcuts();
    initSystemTrayIcon();
    initDBusInterface();
    loadSettings();
    // pop_->setAsnormalWindow(true);
}

MyApplication::~MyApplication() {
    delete setting_window_;
    delete tray_;
    delete pop_;
    delete translator_;
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
    shortcut_act_ = new QAction(tr("translate selection"), this);
    shortcut_act_->setObjectName("io.github.glaumar.PopTranslate");
    setShortcut(setting_window_->shortcuts());
    connect(shortcut_act_, &QAction::triggered, this, &MyApplication::showPop);
    connect(setting_window_,
            &SettingWindow::shortcutChanged,
            this,
            &MyApplication::setShortcut);
}

void MyApplication::showPop(bool unuse) {
    Q_UNUSED(unuse);
    if (pop_->isVisible() && !pop_->isNormalWindow()) {
        pop_->hide();
        return;
    }
    pop_->translate(clipboard_->text(QClipboard::Selection));
    pop_->show();
}

void MyApplication::initSystemTrayIcon() {
    tray_->setToolTip(tr("PopTranslate"));
    tray_->setIcon(QIcon::fromTheme("io.github.glaumar.PopTranslate"));

    // set systemtray menu
    QMenu *menu = new QMenu(setting_window_);

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
            [this]() { this->showPop(true); });

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
}

void MyApplication::trayActivated(QSystemTrayIcon::ActivationReason reason) {
    Q_UNUSED(reason);
    bool is_visible = setting_window_->isVisible();
    setting_window_->setVisible(!is_visible);
}

bool MyApplication::setShortcut(const QList<QKeySequence> &shortcuts) {
    KGlobalAccel::self()->removeAllShortcuts(shortcut_act_);
    bool ret = KGlobalAccel::setGlobalShortcut(shortcut_act_, shortcuts);
    for (auto shortcut : shortcuts) {
        if (shortcut.isEmpty()) continue;
        qDebug() << tr("Bind global shortcuts (%1) for translate selection %2")
                        .arg(shortcut.toString(),
                             ret ? tr("success") : tr("failed"));
    }

    return ret;
}
