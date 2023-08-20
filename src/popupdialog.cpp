#include "popupdialog.h"

#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/registry.h>
#include <KWayland/Client/surface.h>
#include <KWindowSystem/kwindowsystem.h>

#include <KWindowEffects>
#include <QDebug>
#include <QFontMetrics>
#include <QGraphicsOpacityEffect>
#include <QMediaPlaylist>
#include <QOnlineTts>
#include <QStyle>

PopupDialog::PopupDialog(QWidget *parent)
    : QWidget(parent),
      plasmashell_(nullptr),
      ui(new Ui::PopupDialog),
      indicator_(new PageIndicator(this)),
      btn_prev_(new QPushButton(QIcon::fromTheme("go-previous"), "", this)),
      btn_next_(new QPushButton(QIcon::fromTheme("go-next"), "", this)),
      animation_duration_(150),
      animation_group_all_(new QSequentialAnimationGroup(this)),
      animation_group1_(new QParallelAnimationGroup(this)),
      animation_group2_(new QParallelAnimationGroup(this)),
      player_(new QMediaPlayer(this)),
      speak_after_translate_(false) {
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
    initAnimation();
    initStateMachine();
    initTts();

    // show first translate result
    connect(this, &PopupDialog::translateResultsAvailable, [this](int index) {
        if (index == 0) {
            result_index_ = index;
            showTranslateResult(translate_results_.at(result_index_));
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
    clear();
    ui->src_plain_text_edit->setPlainText(text);

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

    auto title_font = font;
    title_font.setPointSizeF(font.pointSizeF() * 0.75);
    ui->title_label->setFont(title_font);
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
    if (metrics.horizontalAdvance(s_text) > ui->title_label->width()) {
        // s_text is too long, so it is truncated
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
    showFloatButton(event->pos());
}

bool PopupDialog::event(QEvent *event) {
    // show menu
    if (event->type() == QEvent::ContextMenu) {
        context_menu_.popup(QCursor::pos());
        return true;
    }

    // hide window
    if (event->type() == QEvent::Leave && context_menu_.isHidden()) {
        if (isNormalWindow()) {
            btn_next_->hide();
            btn_prev_->hide();
        } else {
            this->hide();
        }
        return true;
    }

    if (event->type() == QEvent::Hide) {
        translator_.abort();
        player_->stop();
        btn_prev_->hide();
        btn_next_->hide();
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

        // this->setWindowFlags(Qt::Window );
        // KWindowSystem::setType(pop_window->winId(), NET::WindowType::Normal);
        // KWindowSystem::setState(pop_window->winId(), NET::KeepAbove);

        // blur window Behind
        KWindowEffects::enableBlurBehind(pop_window, setting_.enable_blur);
        plasmaSurface->openUnderCursor();
        plasmaSurface->setSkipTaskbar(!isNormalWindow());
        plasmaSurface->setSkipSwitcher(!isNormalWindow());
        // plasmaSurface->setRole(KWayland::Client::PlasmaShellSurface::Role::Panel);
        // plasmaSurface->setPanelBehavior(KWayland::Client::PlasmaShellSurface::PanelBehavior::AlwaysVisible);
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
    int width = style()->pixelMetric(QStyle::PM_LargeIconSize);

    QSize icon_size(width, width);
    QSize btn_size(width * 2, width * 2);

    btn_prev_->setIconSize(icon_size);
    btn_next_->setIconSize(icon_size);

    btn_prev_->setFixedSize(btn_size);
    btn_next_->setFixedSize(btn_size);

    QString style(
        "QPushButton {background-color: palette(Button); border: none;"
        "border-radius: 48;}"
        "QPushButton:hover { background-color: palette(Midlight); }"
        "QPushButton:pressed { background-color: palette(Highlight); }");

    btn_prev_->setStyleSheet(style);
    btn_next_->setStyleSheet(style);

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

void PopupDialog::showFloatButton(QPoint cursor_pos) {
    // Show prev/next button when the mouse is close to the window edge
    int x = cursor_pos.x();
    const int window_width = this->width();
    const qreal trigger_area = 0.15;
    if (x < window_width * trigger_area && hasPrevResult() &&
        this->underMouse()) {
        btn_prev_->show();
    } else if (x > window_width * (1 - trigger_area) && hasNextResult() &&
               this->underMouse()) {
        btn_next_->show();
    } else {
        btn_prev_->hide();
        btn_next_->hide();
    }
}

void PopupDialog::initPageIndicator() {
    indicator_->initPages(0);
    indicator_->setAnimationDuration(animation_duration_);
    ui->tools_widget->layout()->addWidget(indicator_);
    ui->tools_widget->layout()->setAlignment(indicator_,
                                             Qt::AlignRight | Qt::AlignHCenter);

    auto hlayout = dynamic_cast<QHBoxLayout *>(ui->tools_widget->layout());
    hlayout->setStretchFactor(indicator_, 1);
    hlayout->setStretchFactor(ui->title_label, 3);
    hlayout->setStretchFactor(ui->pronounce_button, 1);

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

void PopupDialog::initAnimation() {
    animation_title_1_ =
        new QPropertyAnimation(ui->title_label, "geometry", this);
    animation_title_2_ =
        new QPropertyAnimation(ui->title_label, "geometry", this);
    animation_translation_1_ =
        new QPropertyAnimation(ui->trans_text_edit, "geometry", this);
    animation_translation_2_ =
        new QPropertyAnimation(ui->trans_text_edit, "geometry", this);

    animation_group1_->addAnimation(animation_title_1_);
    animation_group1_->addAnimation(animation_translation_1_);
    animation_group2_->addAnimation(animation_title_2_);
    animation_group2_->addAnimation(animation_translation_2_);
    animation_group_all_->addAnimation(animation_group1_);
    animation_group_all_->addAnimation(animation_group2_);

    setEffectAnimation(ui->title_label);
    setEffectAnimation(ui->trans_text_edit);
}

void PopupDialog::setEffectAnimation(QWidget *w) {
    // Fade in and out effect
    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(this);
    w->setGraphicsEffect(effect);
    QPropertyAnimation *animation_effects_1 =
        new QPropertyAnimation(effect, "opacity");
    animation_effects_1->setDuration(animation_duration_);
    animation_effects_1->setStartValue(1.0);
    animation_effects_1->setEndValue(0.0);

    QPropertyAnimation *animation_effects_2 =
        new QPropertyAnimation(effect, "opacity");
    animation_effects_2->setDuration(animation_duration_);
    animation_effects_2->setStartValue(0.0);
    animation_effects_2->setEndValue(1.0);

    animation_group1_->addAnimation(animation_effects_1);
    animation_group2_->addAnimation(animation_effects_2);
}

void PopupDialog::startAnimationNext() {
    QRect origin, origin_left, origin_right;

    origin = origin_left = origin_right = ui->title_label->geometry();
    origin_left.moveRight(origin.x() + origin.width() * 0.7);
    origin_right.moveLeft(origin.x() + origin.width() * 0.3);

    animation_title_1_->setStartValue(origin);
    animation_title_1_->setEndValue(origin_left);
    animation_title_1_->setDuration(animation_duration_);

    animation_title_2_->setStartValue(origin_right);
    animation_title_2_->setEndValue(origin);
    animation_title_2_->setDuration(animation_duration_);

    origin = origin_left = origin_right = ui->trans_text_edit->geometry();
    origin_left.moveRight(-1);
    origin_right.moveLeft(ui->trans_text_edit->width());

    animation_translation_1_->setStartValue(origin);
    animation_translation_1_->setEndValue(origin_left);
    animation_translation_1_->setDuration(animation_duration_);

    animation_translation_2_->setStartValue(origin_right);
    animation_translation_2_->setEndValue(origin);
    animation_translation_2_->setDuration(animation_duration_);

    animation_group_all_->start();
}

void PopupDialog::startAnimationPrev() {
    QRect origin, origin_left, origin_right;
    origin = origin_left = origin_right = ui->title_label->geometry();
    origin_left.moveRight(origin.x() + origin.width() * 0.7);
    origin_right.moveLeft(origin.x() + origin.width() * 0.3);

    animation_title_1_->setStartValue(origin);
    animation_title_1_->setEndValue(origin_right);
    animation_title_1_->setDuration(animation_duration_);

    animation_title_2_->setStartValue(origin_left);
    animation_title_2_->setEndValue(origin);
    animation_title_2_->setDuration(animation_duration_);

    origin = origin_left = origin_right = ui->trans_text_edit->geometry();
    origin_left.moveRight(-1);
    origin_right.moveLeft(ui->trans_text_edit->width());

    animation_translation_1_->setStartValue(origin);
    animation_translation_1_->setEndValue(origin_right);
    animation_translation_1_->setDuration(animation_duration_);

    animation_translation_2_->setStartValue(origin_left);
    animation_translation_2_->setEndValue(origin);
    animation_translation_2_->setDuration(animation_duration_);

    animation_group_all_->start();
}

void PopupDialog::initStateMachine() {
    QState *resultUnavailable = new QState();
    QState *resultAvailable = new QState();
    QState *requestNextResult = new QState();
    QState *requestPrevResult = new QState();
    QState *readyShowResult = new QState();

    result_state_machine_.addState(resultUnavailable);
    result_state_machine_.addState(resultAvailable);
    result_state_machine_.addState(requestNextResult);
    result_state_machine_.addState(requestPrevResult);
    result_state_machine_.addState(readyShowResult);

    result_state_machine_.setInitialState(resultUnavailable);

    resultUnavailable->addTransition(this,
                                     &PopupDialog::translateResultsAvailable,
                                     resultAvailable);

    resultAvailable->addTransition(btn_prev_,
                                   &QPushButton::clicked,
                                   requestPrevResult);

    resultAvailable->addTransition(btn_next_,
                                   &QPushButton::clicked,
                                   requestNextResult);

    requestNextResult->addTransition(animation_group1_,
                                     &QAbstractAnimation::finished,
                                     readyShowResult);

    requestPrevResult->addTransition(animation_group1_,
                                     &QAbstractAnimation::finished,
                                     readyShowResult);

    readyShowResult->addTransition(animation_group2_,
                                   &QAbstractAnimation::finished,
                                   resultAvailable);

    readyShowResult->addTransition(this,
                                   &PopupDialog::cleared,
                                   resultUnavailable);

    connect(requestNextResult, &QState::entered, [this] {
        if (hasNextResult()) {
            result_index_++;
            btn_next_->hide();
            startAnimationNext();
        }
    });

    connect(requestPrevResult, &QState::entered, [this] {
        if (hasPrevResult()) {
            result_index_--;
            btn_prev_->hide();
            startAnimationPrev();
        }
    });

    connect(readyShowResult, &QState::entered, [this] {
        showTranslateResult(translate_results_.at(result_index_));
        showFloatButton(QCursor::pos());

        // auto copy translation
        if (setting_.enable_auto_copy_translation) {
            copyTranslation();
        }
    });

    result_state_machine_.start();
}

void PopupDialog::initTts() {
    QMediaPlaylist *playlist = new QMediaPlaylist(player_);
    player_->setPlaylist(playlist);

    connect(ui->pronounce_button, &QToolButton::clicked, [this](bool checked) {
        Q_UNUSED(checked);
        if (translator_.sourceLanguage() == QOnlineTranslator::Auto &&
            translator_.isRunning()) {
            // The language may not have been recognized yet, set this flag and
            // wait for the QOnlineTranslator::finished signal to try again
            speak_after_translate_ = true;
        } else {
            speakText(sourceText(), translator_.sourceLanguage());
        }
    });

    connect(&translator_, &QOnlineTranslator::finished, [this] {
        if (translator_.error() == QOnlineTranslator::NoError) {
            if (speak_after_translate_) {
                speakText(sourceText(), translator_.sourceLanguage());
                speak_after_translate_ = false;
            }
        }
    });
}

void PopupDialog::prepareTextAudio(const QString &text,
                                   QOnlineTranslator::Language lang) {

    if (player_->playlist()->mediaCount() > 0) {
        return;
    }

    QOnlineTts tts;
    auto engine = setting_.translate_engine;
    if (engine != QOnlineTranslator::Engine::Google &&
        engine != QOnlineTranslator::Engine::Yandex) {
        engine = QOnlineTranslator::Engine::Google;
    }

    tts.generateUrls(text, engine, lang);

    if (tts.error() != QOnlineTts::NoError) {
        qWarning() << tr("TTS: %1").arg(tts.errorString());
        return;
    }

    // playlist->clear();
    player_->playlist()->addMedia(tts.media());
}

void PopupDialog::speakText(const QString &text,
                            QOnlineTranslator::Language lang) {
    prepareTextAudio(text, lang);
    player_->play();
}

void PopupDialog::clear() {
    ui->title_label->setText("PopTranslate");
    ui->trans_text_edit->clear();
    ui->src_plain_text_edit->clear();

    translate_results_.clear();
    result_index_ = -1;
    indicator_->clear();

    player_->playlist()->clear();

    emit cleared();
}
