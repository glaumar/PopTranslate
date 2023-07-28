#include "popupdialog.h"

#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/registry.h>
#include <KWayland/Client/surface.h>
#include <KWindowSystem/kwindowsystem.h>

#include <KWindowEffects>
#include <QClipboard>
#include <QDebug>
#include <QGraphicsOpacityEffect>

PopupDialog::PopupDialog(QWidget *parent)
    : QDialog(parent), plasmashell_(nullptr), ui(new Ui::PopupDialog) {
    ui->setupUi(this);
    setNormalWindow(false);

    initContextMenu();
    setOpacity(setting_.opacity);
    setFont(setting_.font);
    initWaylandConnection();
    initTranslator();
    initDictionaries();

    // show first translate result
    connect(this, &PopupDialog::translateResultsAvailable, [this](int index) {
        if (index == 0) {
            current_translate_result_ = translate_results_.cbegin();
            ui->trans_text_edit->setText(*current_translate_result_);
        }
    });
}

PopupDialog::~PopupDialog() { delete ui; }

void PopupDialog::translate(const QString &text) {
    if (text.isEmpty()) {
        return;
    }

    if (translator_.isRunning()) {
        qDebug() << tr("Translate: Abort previous translation");
        translator_.abort();
    }

    qDebug()
        << tr("Translate: Engine: %1, Target language: %2, Source text: %3")
               .arg(DefaultSettings::enumValueToKey(setting_.translate_engine),
                    DefaultSettings::enumValueToKey(setting_.target_language_1),
                    text);

    ui->src_plain_text_edit->setPlainText(text);
    ui->trans_text_edit->clear();

    // clear last translate_results
    translate_results_.clear();
    current_translate_result_ = translate_results_.cend();

    dicts_.lookupAsync(text);
    translator_.translate(text,
                          setting_.translate_engine,
                          setting_.target_language_1);
}

void PopupDialog::retranslate() {
    const QString text = ui->src_plain_text_edit->toPlainText();
    translate(text);
}

void PopupDialog::setNormalWindow(bool enable) {
    if (enable) {
        this->setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);
        // KWindowSystem::setState(this->winId(), NET::SkipTaskbar |
        //                                            NET::SkipSwitcher |
        //                                            NET::SkipPager);
        // KWindowSystem::setState(this->winId(), NET::KeepAbove);

    } else {
        this->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    }
    flag_normal_window_ = enable;
}

void PopupDialog::setTranslateEngine(QOnlineTranslator::Engine engine) {
    qDebug() << tr("Translate: Change translate engine: %1")
                    .arg(DefaultSettings::enumValueToKey(engine));
    setting_.translate_engine = engine;
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
            setting_.target_language_1 = lang;
            retranslate();
        });
        qDebug() << tr("Translate: Add target language to context menu: %1")
                        .arg(DefaultSettings::enumValueToKey(lang));
    }

    setting_.target_language_1 = languages.at(0);
    target_languages_group->actions().at(0)->setChecked(true);
    context_menu_.addActions(target_languages_group->actions());
}

void PopupDialog::setFont(const QFont &font) {
    setting_.font = font;
    ui->trans_text_edit->setFont(font);
    ui->src_plain_text_edit->setFont(font);
}

void PopupDialog::setOpacity(qreal opacity) {
    setting_.opacity = opacity;
    auto opacity_effect = new QGraphicsOpacityEffect(this);
    opacity_effect->setOpacity(opacity);
    this->setGraphicsEffect(opacity_effect);
    this->setAutoFillBackground(true);
}

void PopupDialog::enableBlur(bool enable) { setting_.enable_blur = enable; }

void PopupDialog::setDictionaries(const QStringList &dicts) {
    dicts_.addDicts(dicts);
}

void PopupDialog::removeDictionaries(const QStringList &dicts) {
    dicts_.removeDicts(dicts);
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
        KWindowEffects::enableBlurBehind(this->windowHandle(), false);
        this->hide();
        translator_.abort();
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

        // blur window Behind
        KWindowEffects::enableBlurBehind(pop_window, setting_.enable_blur);
        plasmaSurface->openUnderCursor();
        plasmaSurface->setSkipTaskbar(!isNormalWindow());
        plasmaSurface->setSkipSwitcher(!isNormalWindow());
        pop_window->removeEventFilter(this);
    }
    return ret;
}

void PopupDialog::initContextMenu() {
    // Copy translation
    context_menu_.addAction(
        QIcon::fromTheme("edit-copy"),
        tr("Copy translation"),
        this,
        [this] {
            QClipboard *clipboard = QApplication::clipboard();
            clipboard->setText(ui->trans_text_edit->toPlainText());
        });

    // Source Text
    action_source_text_ =
        context_menu_.addAction(QIcon::fromTheme("texture"), tr("Source Text"));
    action_source_text_->setCheckable(true);
    connect(action_source_text_, &QAction::triggered, this, [this](bool state) {
        if (state) {
            QSize new_size(this->size().width(), this->size().height() * 2);
            this->resize(new_size);
        } else {
            QSize new_size(this->size().width(), this->size().height() / 2);
            this->resize(new_size);
        }
        ui->src_plain_text_edit->setVisible(state);
    });
    setSrcTextEditVisible(false);

    // Pin the window
    QAction *action_pin_windows =
        context_menu_.addAction(QIcon::fromTheme("window-pin"),
                                tr("Pin the window"));
    action_pin_windows->setCheckable(true);
    connect(action_pin_windows, &QAction::triggered, this, [this](bool state) {
        setNormalWindow(state);
        hide();
        show();
    });
    action_pin_windows->setChecked(false);

    // Settings
    context_menu_.addAction(QIcon::fromTheme("settings-configure"),
                            tr("Settings"),
                            [this] { emit settingsActionTriggered(); });

    // Translate_engine
    engine_menu_.setIcon(QIcon::fromTheme("search"));
    engine_menu_.setTitle(tr("Translate Engine"));
    auto engine_group = new QActionGroup(&engine_menu_);
    engine_group->setExclusive(true);

    QMetaEnum engines_enum = QMetaEnum::fromType<QOnlineTranslator::Engine>();
    for (int i = 0; i < engines_enum.keyCount(); i++) {
        // TODO: Add support for LibreTranslate and Lingva
        // Disable LibreTranslate and Lingva
        if (engines_enum.value(i) ==
                QOnlineTranslator::Engine::LibreTranslate ||
            engines_enum.value(i) == QOnlineTranslator::Engine::Lingva) {
            continue;
        }

        auto engine = engines_enum.key(i);
        auto action = engine_group->addAction(engine);
        action->setCheckable(true);
        action->setChecked(setting_.translate_engine == engines_enum.value(i));

        connect(action, &QAction::triggered, [this, engines_enum, i]() {
            setting_.translate_engine =
                static_cast<QOnlineTranslator::Engine>(engines_enum.value(i));
            retranslate();
        });
    }
    engine_menu_.addActions(engine_group->actions());
    context_menu_.addMenu(&engine_menu_);

    // Target Languages
    context_menu_.addSeparator();
    setTargetLanguages({setting_.target_language_1,
                        setting_.target_language_2,
                        setting_.target_language_3});
}

void PopupDialog::initWaylandConnection() {
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

void PopupDialog::initTranslator() {
    connect(&translator_, &QOnlineTranslator::finished, [this] {
        if (translator_.error() == QOnlineTranslator::NoError) {
            qDebug() << tr("Translate Success: %1").arg(translator_.source());
            translate_results_.append(translator_.translation());
            emit translateResultsAvailable(translate_results_.size() - 1);
        } else {
            auto error_msg =
                tr("Failed Translate: %1 (%2)")
                    .arg(translator_.errorString(), translator_.source());
            if (this->isVisible() &&
                translator_.errorString() != QString("Operation canceled")) {
                ui->trans_text_edit->setText(error_msg);
            }
            qWarning() << error_msg;
        }
    });
}

void PopupDialog::initDictionaries() {
    // lookup word in dictionaries
    connect(
        &dicts_,
        &Dictionaries::found,
        this,
        [this](QString result) {
            translate_results_.append(result);
            emit translateResultsAvailable(translate_results_.size() - 1);
        },
        Qt::QueuedConnection);
}