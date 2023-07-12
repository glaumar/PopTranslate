#include "popupdialog.h"

#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/registry.h>
#include <KWayland/Client/surface.h>
#include <KWindowSystem/kwindowsystem.h>

#include <QClipboard>
#include <QDebug>

#include "ui_popupdialog.h"

PopupDialog::PopupDialog(QWidget *parent)
    : QDialog(parent), plasmashell_(nullptr), ui(new Ui::PopupDialog) {
    ui->setupUi(this);
    setNormalWindow(false);

    current_translate_engine_ = default_.translate_engine;
    current_target_language_ = default_.target_language_1;
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
    connect(registry,
            &KWayland::Client::Registry::plasmaShellAnnounced,
            this,
            [registry, this](quint32 name, quint32 version) {
                if (!plasmashell_) {
                    plasmashell_ = registry->createPlasmaShell(name, version);
                }
            });
    registry->create(connection);
    registry->setup();
}

PopupDialog::~PopupDialog() { delete ui; }

void PopupDialog::translate(const QString &text) {
    qDebug()
        << QString("Engine: %1, Target language: %2, Source text: %3")
               .arg(DefaultSettings::enumValueToKey(current_translate_engine_),
                    DefaultSettings::enumValueToKey(current_target_language_),
                    text);

    ui->src_plain_text_edit->setPlainText(text);
    ui->trans_text_edit->clear();

    translator_.translate(text,
                          current_translate_engine_,
                          current_target_language_);
    QObject::connect(&translator_, &QOnlineTranslator::finished, [this] {
        if (translator_.error() == QOnlineTranslator::NoError) {
            ui->trans_text_edit->setText(translator_.translation());
        } else {
            ui->trans_text_edit->setText(translator_.errorString());
            qWarning() << QString("Failed Translate: %1")
                            .arg(translator_.errorString());
        }
    });
}

void PopupDialog::retranslate() {
    const QString text = ui->src_plain_text_edit->toPlainText();
    translate(text);
}

bool PopupDialog::isNormalWindow() const { return flag_normal_window_; }

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
    flag_normal_window_ = on;
}

void PopupDialog::setTranslateEngine(QOnlineTranslator::Engine engine) {
    qDebug() << QString("Set translate engine: %1")
                    .arg(DefaultSettings::enumValueToKey(engine));
    current_translate_engine_ = engine;
    engine_menu_.actions().at(engine)->setChecked(true);
}

void PopupDialog::setTargetLanguages(
    QVector<QOnlineTranslator::Language> languages) {
    static auto target_languages_group = new QActionGroup(&context_menu_);

    // remove old actions
    for (auto action : target_languages_group->actions()) {
        context_menu_.removeAction(action);
        target_languages_group->removeAction(action);
    }

    target_languages_group->setExclusive(true);

    // add new actions
    for (auto lang : languages) {
        auto action = target_languages_group->addAction(
            DefaultSettings::enumValueToKey(lang));
        action->setCheckable(true);
        connect(action, &QAction::triggered, [this, lang]() {
            current_target_language_ = lang;
            retranslate();
        });
        qDebug() << QString("Add target language to context menu: %1")
                        .arg(DefaultSettings::enumValueToKey(lang));
    }

    current_target_language_ = languages.at(0);
    target_languages_group->actions().at(0)->setChecked(true);
    context_menu_.addActions(target_languages_group->actions());
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
        auto plasmaSurface = plasmashell_->createSurface(surface, pop_window);
        plasmaSurface->openUnderCursor();
        plasmaSurface->setSkipTaskbar(!isNormalWindow());
        plasmaSurface->setSkipSwitcher(!isNormalWindow());
        pop_window->removeEventFilter(this);
    }
    return ret;
}

void PopupDialog::initContextMenu() {
    context_menu_.addAction(
        QIcon::fromTheme("edit-copy"),
        tr("Copy translation"),
        this,
        [this] {
            QClipboard *clipboard = QApplication::clipboard();
            clipboard->setText(ui->trans_text_edit->toPlainText());
        });

    QAction *action_source_text =
        context_menu_.addAction(QIcon::fromTheme("texture"), tr("Source Text"));
    action_source_text->setCheckable(true);
    connect(action_source_text, &QAction::triggered, this, [this](bool state) {
        ui->src_plain_text_edit->setVisible(state);
    });
    ui->src_plain_text_edit->setVisible(false);
    action_source_text->setChecked(false);

    context_menu_.addAction(QIcon::fromTheme("settings-configure"),
                            tr("Settings"));

    // translate_engine
    engine_menu_.setTitle(tr("Translate Engine"));
    auto engine_group = new QActionGroup(&engine_menu_);
    engine_group->setExclusive(true);

    QMetaEnum engines_enum = QMetaEnum::fromType<QOnlineTranslator::Engine>();
    for (int i = 0; i < engines_enum.keyCount(); i++) {
        auto engine = engines_enum.key(i);
        auto action = engine_group->addAction(engine);
        action->setCheckable(true);
        action->setChecked(current_translate_engine_ == engines_enum.value(i));

        connect(action, &QAction::triggered, [this, engines_enum, i]() {
            current_translate_engine_ =
                static_cast<QOnlineTranslator::Engine>(engines_enum.value(i));
            retranslate();
        });
    }
    engine_menu_.addActions(engine_group->actions());
    context_menu_.addMenu(&engine_menu_);

    // target_language
    context_menu_.addSeparator();
    setTargetLanguages({default_.target_language_1,
                        default_.target_language_2,
                        default_.target_language_3});
}