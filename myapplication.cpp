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
      pop(nullptr),
      tray(nullptr),
      clipboard(KSystemClipboard::instance()){
    setApplicationName("PopTranslate");
    initGlobalShortcuts();
    initSystemTrayIcon();
    initDBusInterface();
    // pop.setAsnormalWindow(true);
}

bool MyApplication::initGlobalShortcuts() {
    QAction *act = new QAction(tr("translate selection"), this);
    QKeySequence kseq(Qt::META | Qt::Key_G);
    act->setObjectName(tr("io.github.glaumar.PopTranslate"));
    bool ret = KGlobalAccel::setGlobalShortcut(act, kseq);

    if (!ret) {
        qDebug() << tr("Failed to bind global shortcuts for translate selection");
        return ret;
    }

    connect(act, &QAction::triggered, this, &MyApplication::showPop);
    return ret;
}

void MyApplication::showPop(bool unuse) {
    if (pop.isVisible() && !pop.isNormalWindow()) {
        pop.hide();
        return;
    }
    pop.SetTransWords(clipboard->text(QClipboard::Selection));
    pop.show();
}

void MyApplication::initSystemTrayIcon() {
    tray.setToolTip(tr("PopTranslate"));
    tray.setIcon(QIcon::fromTheme("io.github.glaumar.PopTranslate"));

    // set systemtray menu
    QMenu *menu = new QMenu(&pop); // TODO: use a better parent
    menu->addAction(QIcon::fromTheme("settings-configure"),tr("Settings"), this, [this]() { // TODO: show settings window
    });
    menu->addAction(QIcon::fromTheme("application-exit"),("Exit"), this, [this]() {
        tray.setVisible(false);
        this->quit();
    });
    tray.setContextMenu(menu);

    connect(&tray, &QSystemTrayIcon::activated, this,
            &MyApplication::trayActivated);
    tray.show();
}

void MyApplication::initDBusInterface() {
    PopTranslateDBus::instance()->registerService();
    connect(PopTranslateDBus::instance(), &PopTranslateDBus::receivedTranslateSelection, this, [this]() {
        this->showPop(true);
    });

    connect(PopTranslateDBus::instance(), &PopTranslateDBus::receivedTranslate, this, [this](const QString& text) {
        this->pop.SetTransWords(text);
        this->pop.show();
    });
}

void MyApplication::trayActivated(QSystemTrayIcon::ActivationReason reason) {
    // TODO: Show settings window
}
