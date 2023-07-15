#include "myapplication.h"

#include <KWindowSystem/kwindowsystem.h>
#include <kglobalaccel.h>

#include <QAction>
#include <QClipboard>
#include <QCloseEvent>
#include <QDebug>
#include <QKeySequence>
#include <QMessageBox>
#include <QSystemTrayIcon>

#include "poptranslate_dbus.h"

MyApplication::MyApplication(int &argc, char **argv)
    : QApplication(argc, argv),
      pop_(nullptr),
      tray_(nullptr),
      setting_window_(nullptr),
      clipboard_(KSystemClipboard::instance()) {
    setApplicationName("PopTranslate");
    initGlobalShortcuts();
    initSystemTrayIcon();
    initDBusInterface();
    loadSettings();
    // pop_.setAsnormalWindow(true);
}

bool MyApplication::initGlobalShortcuts() {
    QAction *act = new QAction(tr("translate selection"), this);
    QKeySequence kseq(Qt::META | Qt::Key_G);
    act->setObjectName("io.github.glaumar.PopTranslate");
    bool ret = KGlobalAccel::setGlobalShortcut(act, kseq);

    if (!ret) {
        qDebug() << tr(
            "Failed to bind global shortcuts for translate selection");
        return ret;
    }

    connect(act, &QAction::triggered, this, &MyApplication::showPop);
    return ret;
}

void MyApplication::showPop(bool unuse) {
    Q_UNUSED(unuse);
    if (pop_.isVisible() && !pop_.isNormalWindow()) {
        pop_.hide();
        return;
    }
    pop_.translate(clipboard_->text(QClipboard::Selection));
    pop_.show();
}

void MyApplication::initSystemTrayIcon() {
    tray_.setToolTip(tr("PopTranslate"));
    tray_.setIcon(QIcon::fromTheme("io.github.glaumar.PopTranslate"));

    // set systemtray menu
    QMenu *menu = new QMenu(&setting_window_);

    menu->addAction(QIcon::fromTheme("settings-configure"),
                    tr("Settings"),
                    this,
                    [this]() { setting_window_.show(); });

    menu->addAction(QIcon::fromTheme("application-exit"),
                    tr("Exit"),
                    this,
                    [this]() {
                        tray_.setVisible(false);
                        this->quit();
                    });
    tray_.setContextMenu(menu);

    connect(&tray_,
            &QSystemTrayIcon::activated,
            this,
            &MyApplication::trayActivated);
    tray_.show();
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
                this->pop_.translate(text);
                this->pop_.show();
            });
}

void MyApplication::loadSettings() {
    connect(&setting_window_,
            &SettingWindow::translateEngineChanged,
            &pop_,
            &PopupDialog::setTranslateEngine);

    connect(&setting_window_,
            &SettingWindow::targetLanguagesChanged,
            &pop_,
            &PopupDialog::setTargetLanguages);

    connect(&setting_window_,
            &SettingWindow::fontChanged,
            &pop_,
            &PopupDialog::setFont);

    connect(&setting_window_,
            &SettingWindow::opacityChanged,
            &pop_,
            &PopupDialog::setOpacity);

    connect(&setting_window_,
            &SettingWindow::triggerBlurEffect,
            &pop_,
            &PopupDialog::enableBlur);

    // load settings for popupdialog
    pop_.setTranslateEngine(setting_window_.translateEngine());
    pop_.setTargetLanguages(setting_window_.targetLanguages());
    pop_.setFont(setting_window_.font());
    pop_.setOpacity(setting_window_.opacity());
    pop_.enableBlur(setting_window_.isEnableBlur());
}

void MyApplication::trayActivated(QSystemTrayIcon::ActivationReason reason) {
    Q_UNUSED(reason);
    bool is_visible = setting_window_.isVisible();
    setting_window_.setVisible(!is_visible);
}
