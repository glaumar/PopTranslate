#include "mainwindow.h"

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

#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      pop(new PopupDialog(this)),
      tray(new QSystemTrayIcon(this)),
      clipboard(KSystemClipboard::instance()),
      ui(new Ui::MainWindow) {
    ui->setupUi(this);
    initGlobalShortcuts();
    initSystemTrayIcon();
    initDBusInterface();

    // pop->setAsnormalWindow(true);
}

MainWindow::~MainWindow() { delete ui; }

bool MainWindow::initGlobalShortcuts() {
    QAction *act = new QAction(tr("translate selection"), this);
    QKeySequence kseq(Qt::META | Qt::Key_G);
    act->setObjectName(tr("io.github.glaumar.PopTranslate"));
    bool ret = KGlobalAccel::setGlobalShortcut(act, kseq);

    if (!ret) {
        qDebug() << tr("Failed to bind global shortcuts for translate selection");
        return ret;
    }

    connect(act, &QAction::triggered, this, &MainWindow::showPop);
    return ret;
}

void MainWindow::showPop(bool unuse) {
    if (pop->isVisible() && !pop->isNormalWindow()) {
        pop->hide();
        return;
    }

    pop->SetTransWords(clipboard->text(QClipboard::Selection));
    pop->show();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (tray->isVisible()) {
        hide();
        event->ignore();
    } else {
        QMainWindow::closeEvent(event);
    }
}

void MainWindow::initSystemTrayIcon() {
    tray->setToolTip(tr("PopTranslate"));
    tray->setIcon(QIcon::fromTheme("io.github.glaumar.PopTranslate"));

    // set systemtray menu
    QMenu *menu = new QMenu(this);
    menu->addAction(QIcon::fromTheme("settings-configure"),tr("Settings"), this, [this]() { this->show(); });
    menu->addAction(QIcon::fromTheme("application-exit"),("Exit"), this, [this]() {
        tray->setVisible(false);
        this->close();
    });
    tray->setContextMenu(menu);

    connect(tray, &QSystemTrayIcon::activated, this,
            &MainWindow::trayActivated);
    tray->show();
}

void MainWindow::initDBusInterface() {
    PopTranslateDBus::instance()->registerService();
    connect(PopTranslateDBus::instance(), &PopTranslateDBus::receivedTranslateSelection, this, [this]() {
        this->showPop(true);
    });

    connect(PopTranslateDBus::instance(), &PopTranslateDBus::receivedTranslate, this, [this](const QString& text) {
        this->pop->SetTransWords(text);
        this->pop->show();
    });
}

void MainWindow::trayActivated(QSystemTrayIcon::ActivationReason reason) {
    setVisible(!isVisible());
}
