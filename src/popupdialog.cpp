#include "popupdialog.h"

#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/registry.h>
#include <KWayland/Client/surface.h>
#include <KWindowSystem/kwindowsystem.h>

#include <KWindowEffects>
#include <QDebug>
#include <QFontMetrics>

PopupDialog::PopupDialog(QWidget *parent)
    : QWidget(parent),
      plasmashell_(nullptr),
      ui(new Ui::PopupDialog),
      indicator_(new PageIndicator(this)),
      btn_prev_(new QPushButton(QIcon::fromTheme("go-previous"), "", this)),
      btn_next_(new QPushButton(QIcon::fromTheme("go-next"), "", this)) {
    ui->setupUi(this);
    setNormalWindow(false);

    initContextMenu();
    setOpacity(setting_.opacity);
    setFont(setting_.font);
    initWaylandConnection();
    initTranslator();
    initDictionaries();
    initFloatButton();
    initPageIndicator();

    // show first translate result
    connect(this, &PopupDialog::translateResultsAvailable, [this](int index) {
        if (index == 0) {
            result_index_ = index;
            showTranslateResult(translate_results_.at(result_index_));
        }
    });

    // auto copy translation
    connect(ui->trans_text_edit, &QTextEdit::textChanged, this, [this] {
        if (setting_.enable_auto_copy_translation) {
            copyTranslation();
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

    qDebug() << tr("Translate: Engine: %1, Target language: %2")
                    .arg(DefaultSettings::enumValueToKey(
                             setting_.translate_engine),
                         DefaultSettings::enumValueToKey(
                             setting_.target_language_1));

    ui->src_plain_text_edit->setPlainText(text);
    ui->trans_text_edit->clear();

    // clear last translate_results
    translate_results_.clear();
    result_index_ = -1;
    ui->title_label->setText("PopTranslate");
    indicator_->clear();

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
    // qDebug() << tr("Translate: Change translate engine: %1")
    //                 .arg(DefaultSettings::enumValueToKey(engine));
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
        // qDebug() << tr("Translate: Add target language to context menu: %1")
        //                 .arg(DefaultSettings::enumValueToKey(lang));
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

    QPalette pal = palette();
    QColor background_color = pal.color(QWidget::backgroundRole());
    background_color.setAlpha(static_cast<int>(opacity * 255));
    pal.setColor(QPalette::Window, background_color);
    setPalette(pal);

    QPalette pal_edit = ui->trans_text_edit->palette();
    background_color.setAlpha(0);
    pal_edit.setColor(QPalette::Base, background_color);
    ui->trans_text_edit->setPalette(pal_edit);
    ui->src_plain_text_edit->setPalette(pal_edit);
}

void PopupDialog::enableBlur(bool enable) { setting_.enable_blur = enable; }

void PopupDialog::enableAutoCopyTranslation(bool enable) {
    setting_.enable_auto_copy_translation = enable;
}

void PopupDialog::showTranslateResult(const QPair<QString, QString> &result) {
    auto &[s_text, t_text] = result;

    QFontMetrics metrics(ui->title_label->font());
    QString str = s_text;
    if (metrics.horizontalAdvance(s_text) > ui->title_label->width()) {
        QString str =
            QFontMetrics(ui->title_label->font())
                .elidedText(s_text, Qt::ElideRight, ui->title_label->width());
        ui->title_label->setText(str);
    } else {
        ui->title_label->setText(s_text);
    }

    auto plain_text = QTextDocumentFragment::fromHtml(t_text).toPlainText();
    ui->trans_text_edit->setText(plain_text);
}

void PopupDialog::setDictionaries(const QStringList &dicts) {
    dicts_.setDicts(dicts);
}

void PopupDialog::mouseMoveEvent(QMouseEvent *event) {
    // Show prev/next button when the mouse is close to the window edge
    const int x = event->pos().x();
    const int window_width = this->width();
    const qreal trigger_area = 0.15;
    if (x < window_width * trigger_area && hasPrevResult()) {
        btn_prev_->show();
    } else if (x > window_width * (1 - trigger_area) && hasNextResult()) {
        btn_next_->show();
    } else {
        btn_prev_->hide();
        btn_next_->hide();
    }
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
        // KWindowEffects::enableBlurBehind(this->windowHandle(), false);
        this->hide();
        return true;
    }

    if (event->type() == QEvent::Hide) {
        translator_.abort();
        btn_prev_->hide();
        btn_next_->hide();
        indicator_->clear();
    }

    if (event->type() == QEvent::Show) {
        this->windowHandle()->installEventFilter(this);
    }

    return QWidget::event(event);
}

bool PopupDialog::eventFilter(QObject *filtered, QEvent *event) {
    // show window under mouse cursor on wayland
    const bool ret = QObject::eventFilter(filtered, event);
    auto pop_window = qobject_cast<QWindow *>(filtered);

    if (pop_window && event->type() == QEvent::Expose) {
        if (!pop_window->isVisible()) {
            pop_window->setVisible(true);
        }

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
    // Copy Source Text
    context_menu_.addAction(QIcon::fromTheme("edit-copy"),
                            tr("Copy Source Text"),
                            this,
                            &PopupDialog::copySourceText);

    // Copy translation
    context_menu_.addAction(QIcon::fromTheme("edit-copy"),
                            tr("Copy Translation"),
                            this,
                            &PopupDialog::copyTranslation);

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
        setSrcTextEditVisible(state);
    });
    setSrcTextEditVisible(false);

    // Pin the window
    QAction *action_pin_windows =
        context_menu_.addAction(QIcon::fromTheme("window-pin"), tr("Pin"));
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
            qDebug() << tr("Translate Success");
            QPair<QString, QString> result(setting_.translate_engine_to_str(),
                                           translator_.translation());
            translate_results_.append(result);
            emit translateResultsAvailable(translate_results_.size() - 1);
        } else {
            auto error_msg =
                tr("Failed Translate: %1").arg(translator_.errorString());
            if (this->isVisible() &&
                translator_.errorString() != QString("Operation canceled")) {
                QPair<QString, QString> result(
                    setting_.translate_engine_to_str(),
                    error_msg);
                translate_results_.append(result);
                emit translateResultsAvailable(translate_results_.size() - 1);
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
        [this](QPair<QString, QString> result) {
            translate_results_.append(result);
            emit translateResultsAvailable(translate_results_.size() - 1);
        },
        Qt::QueuedConnection);
}

void PopupDialog::initFloatButton() {
    btn_prev_->setIconSize(QSize(64, 64));
    btn_next_->setIconSize(QSize(64, 64));

    btn_prev_->setFixedSize(96, 96);
    btn_next_->setFixedSize(96, 96);

    auto old_bg_color = palette().color(QWidget::backgroundRole());
    auto new_bg_color = QColor::fromHsv(old_bg_color.hue() - 50,
                                        old_bg_color.saturation(),
                                        old_bg_color.value());

    QString style_template(
        "QPushButton {background-color: rgba(%1, %2, %3, 50); border: none; "
        "border-radius: 48;}"
        "QPushButton:hover { background-color: rgba(%1, %2, %3, 150); }"
        "QPushButton:pressed { background-color: rgba(%1, %2, %3, 100); }");

    auto style = style_template.arg(new_bg_color.red())
                     .arg(new_bg_color.green())
                     .arg(new_bg_color.blue());

    btn_prev_->setStyleSheet(style);
    btn_next_->setStyleSheet(style);

    connect(btn_prev_, &QPushButton::clicked, [this] {
        if (result_index_ > 0) {
            result_index_--;
            showTranslateResult(translate_results_.at(result_index_));
        }
        if (!hasPrevResult()) {
            btn_prev_->hide();
        }
    });

    connect(btn_next_, &QPushButton::clicked, [this] {
        if (result_index_ >= 0 &&
            result_index_ < translate_results_.size() - 1) {
            result_index_++;
            showTranslateResult(translate_results_.at(result_index_));
        }
        if (!hasNextResult()) {
            btn_next_->hide();
        }
    });

    btn_prev_->hide();
    btn_next_->hide();

    QHBoxLayout *layout = new QHBoxLayout(ui->trans_text_edit);
    layout->addWidget(btn_prev_);
    layout->addWidget(btn_next_);
    layout->setAlignment(btn_prev_, Qt::AlignLeft);
    layout->setAlignment(btn_next_, Qt::AlignRight);

    // If mouse tracking is switched on, mouse move events occur even if no
    // mouse button is pressed
    setMouseTracking(true);
    ui->trans_text_edit->setMouseTracking(true);
}

void PopupDialog::initPageIndicator() {
    indicator_->initPages(0);
    ui->tools_widget->layout()->addWidget(indicator_);
    ui->tools_widget->layout()->setAlignment(indicator_,
                                             Qt::AlignRight | Qt::AlignHCenter);

    auto hlayout = dynamic_cast<QHBoxLayout*>(ui->tools_widget->layout());
    hlayout->setStretchFactor(indicator_, 1);
    hlayout->setStretchFactor(ui->title_label, 3);
    
    

    connect(btn_prev_, &QPushButton::clicked, [this] {
        indicator_->prevPage();
    });

    connect(btn_next_, &QPushButton::clicked, [this] {
        indicator_->nextPage();
    });

    connect(this, &PopupDialog::translateResultsAvailable, [this](int index) {
        if (index == 0) {
            indicator_->initPages(1);
        } else {
            indicator_->addPages(1);
        }
    });
}
