#include "popupdialog.h"

#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/registry.h>
#include <KWayland/Client/surface.h>
#include <KWindowSystem/kwindowsystem.h>

#include <QClipboard>
#include <QDebug>

#include "ui_popupdialog.h"

PopupDialog::PopupDialog(QWidget *parent)
    : QDialog(parent), plasmashell(nullptr), ui(new Ui::PopupDialog) {
    ui->setupUi(this);
    setNormalWindow(false);

    initContextMenu();

    // TODO: make poptranslate as a normal window
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
    ui->src_plain_text_edit->setPlainText(words);
    ui->trans_text_edit->clear();
    translator_.translate(words, QOnlineTranslator::Google,
                          QOnlineTranslator::SimplifiedChinese);
    QObject::connect(&translator_, &QOnlineTranslator::finished, [&] {
        if (translator_.error() == QOnlineTranslator::NoError)
            ui->trans_text_edit->setText(translator_.translation());
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
    // show menu
    if (event->type() == QEvent::ContextMenu) {
        context_menu_.popup(QCursor::pos());
        return true;
    }

    // hide window
    if (!isNormalWindow() && event->type() == QEvent::Leave &&
        context_menu_.isHidden()) {
        this->hide();
        return true;
    }

    if (event->type() == QEvent::Show) {
        this->windowHandle()->installEventFilter(this);
    }

    return QDialog::event(event);
}

bool PopupDialog::eventFilter(QObject *filtered, QEvent *event) {
    // show window under mouse cursor on wayland
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

void PopupDialog::initContextMenu() {
    context_menu_.addAction(
        QIcon::fromTheme("edit-copy"), tr("Copy translation"), this, [this] {
            QClipboard *clipboard = QApplication::clipboard();
            clipboard->setText(ui->trans_text_edit->toPlainText());
        });

    QAction *action_source_text =
        context_menu_.addAction(QIcon::fromTheme("texture"), tr("Source text"));
    action_source_text->setCheckable(true);
    connect(action_source_text, &QAction::triggered, this,
            [this](bool state) { ui->src_plain_text_edit->setVisible(state); });
    ui->src_plain_text_edit->setVisible(false);
    action_source_text->setChecked(false);

    context_menu_.addAction(QIcon::fromTheme("settings-configure"),
                            tr("Settings"));
}