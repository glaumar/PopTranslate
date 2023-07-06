#include "popupdialog.h"

#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/registry.h>
#include <KWayland/Client/surface.h>
#include <KWindowSystem/kwindowsystem.h>

#include <QDebug>

#include "ui_popupdialog.h"

PopupDialog::PopupDialog(QWidget *parent)
    : QDialog(parent), plasmashell(nullptr), ui(new Ui::PopupDialog) {
    ui->setupUi(this);
    setNormalWindow(false);

    // TODO: make poptrans as a normal window
    // connect(ui->pin_push_button, &QPushButton::clicked, this, [this]() {
    //     setNormalWindow(!isNormalWindow());
    //     hide();
    //     show();
    // });

    auto registry = new KWayland::Client::Registry(this);
    auto connection =
        KWayland::Client::ConnectionThread::fromApplication(qGuiApp);
    connect(registry, &KWayland::Client::Registry::plasmaShellAnnounced, this,
            [registry, this](quint32 name, quint32 version) {
                if (!plasmashell) {
                    plasmashell = registry->createPlasmaShell(name, version);
                }
            });
    registry->create(connection);
    registry->setup();
}

PopupDialog::~PopupDialog() { delete ui; }

void PopupDialog::SetTransWords(const QString &words) {
    ui->trans_text_edit->clear();
    translator_.translate(words,QOnlineTranslator::Google,QOnlineTranslator::SimplifiedChinese);
    QObject::connect(&translator_, &QOnlineTranslator::finished, [&] {
    if (translator_.error() == QOnlineTranslator::NoError)
        ui->trans_text_edit->setText( translator_.translation());
    else
        ui->trans_text_edit->setText(translator_.errorString());
});

}

bool PopupDialog::isNormalWindow() const { return flag_normal_window; }

void PopupDialog::setNormalWindow(bool on) {
    if (on) {
        this->setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);
        // KWindowSystem::setState(this->winId(), NET::SkipTaskbar |
        //                                            NET::SkipSwitcher |
        //                                            NET::SkipPager);
        // KWindowSystem::setState(this->winId(), NET::KeepAbove);

    } else {
        this->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    }
    flag_normal_window = on;
}

bool PopupDialog::event(QEvent *event) {
    if (!isNormalWindow() && event->type() == QEvent::Leave) {
        this->hide();
        return true;
    }

    if (event->type() == QEvent::Show) {
        this->windowHandle()->installEventFilter(this);
    }

    return QDialog::event(event);
}

bool PopupDialog::eventFilter(QObject *filtered, QEvent *event) {
    const bool ret = QObject::eventFilter(filtered, event);
    auto pop_window = qobject_cast<QWindow *>(filtered);
    if (pop_window && event->type() == QEvent::Expose &&
        pop_window->isVisible()) {
        auto surface = KWayland::Client::Surface::fromWindow(pop_window);
        auto plasmaSurface = plasmashell->createSurface(surface, pop_window);
        plasmaSurface->openUnderCursor();
        plasmaSurface->setSkipTaskbar(!isNormalWindow());
        plasmaSurface->setSkipSwitcher(!isNormalWindow());
        pop_window->removeEventFilter(this);
    }
    return ret;
}
