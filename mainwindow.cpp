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

    // pop->setAsnormalWindow(true);
}

MainWindow::~MainWindow() { delete ui; }

bool MainWindow::initGlobalShortcuts() {
    QAction *act = new QAction(tr("scan popup"), this);
    QKeySequence kseq(Qt::META | Qt::Key_G);
    act->setObjectName(tr("poptrans_popup"));
    // TODO: set ICON
    bool ret = KGlobalAccel::setGlobalShortcut(act, kseq);

    if (!ret) {
        qDebug() << tr("Failed to bind global shortcuts for scan popup");
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
    tray->setToolTip(tr("poptrans"));
    tray->setIcon(QIcon());

    // set systemtray menu
    QMenu *menu = new QMenu(this);
    menu->addAction(tr("Show main window"), this, [this]() { this->show(); });
    menu->addAction(tr("exit"), this, [this]() {
        tray->setVisible(false);
        this->close();
    });
    tray->setContextMenu(menu);

    connect(tray, &QSystemTrayIcon::activated, this,
            &MainWindow::trayActivated);
    tray->show();
}

void MainWindow::trayActivated(QSystemTrayIcon::ActivationReason reason) {
    setVisible(!isVisible());
}
