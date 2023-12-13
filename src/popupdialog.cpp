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
#include <QRadioButton>
#include <QStyle>

#include "lang2iso639.h"
#include "onlinetranslator.h"
#include "poptranslate.h"
#include "qonlinetranslator.h"

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
      animation_group2_(new QParallelAnimationGroup(this)) {
    ui->setupUi(this);
    loadSettings();
    enableMonitorMode(false);
    PopTranslateSettings::instance().setMonitorClipboard(false);
    initBottomButtons();
    initWaylandConnection();
    initFloatButton();
    initPageIndicator();
    initAnimation();
    initStateMachine();

    // show first translate result
    connect(this, &PopupDialog::newResultsAvailable, [this](int index) {
        if (index == 0) {
            result_index_ = index;
            showTranslateResult(translate_results_.at(result_index_));
        }
    });

    // pronounce button
    connect(ui->pronounce_button, &QPushButton::clicked, [this] {
        emit requestSpeak(sourceText());
    });
    connect(ui->pronounce_button_2, &QPushButton::clicked, [this] {
        emit requestSpeak(sourceText());
    });

    // search lineEdit
    ui->search_lineedit->hide();
    connect(ui->search_lineedit,
            &QLineEdit::textChanged,
            [this](const QString &text) {
                if (!text.isEmpty()) {
                    emit requestTranslate(text);
                }
            });

    // src text edit
    enableSrcEditMode(false);
    connect(ui->show_src_toolbutton, &QToolButton::clicked, [this] {
        auto flag = ui->src_plain_text_edit->isVisible();
        enableSrcEditMode(!flag);
    });

    connect(ui->translate_pushbutton, &QPushButton::clicked, [this] {
        enableSrcEditMode(false);
        emit requestTranslate(sourceText());
    });

    // Avoid too small a space causing only "..." to be displayed
    QFontMetrics metrics(ui->title_label->font());
    ui->title_label->setMinimumWidth(
        metrics.horizontalAdvance(ui->title_label->text()));
}

PopupDialog::~PopupDialog() {
    PopTranslateSettings::instance().setPopupWindowSize(this->size());

    delete ui;
}

void PopupDialog::enableMonitorMode(bool enable) {
    if (enable) {
        this->setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);
        // KWindowSystem::setState(this->winId(), NET::SkipTaskbar |
        //                                            NET::SkipSwitcher |
        //                                            NET::SkipPager);
        // KWindowSystem::setState(this->winId(), NET::KeepAbove);
        ui->pin_button->setIcon(QIcon::fromTheme("xapp-unpin-symbolic"));
    } else {
        this->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
        ui->pin_button->setIcon(QIcon::fromTheme("pin"));
    }
}

void PopupDialog::addTranslateResult(const AbstractTranslator::Result &result) {
    translate_results_.append(result);
    emit newResultsAvailable(translate_results_.size() - 1);
}

void PopupDialog::setTargetLanguages(
    QVector<QOnlineTranslator::Language> languages) {
    auto layout = ui->bottom_left->layout();

    // clear old button
    QLayoutItem *child = nullptr;
    while ((child = layout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    if (languages.isEmpty()) {
        return;
    }

    PopTranslateSettings::instance().setActiveTargetLanguage(languages.at(0));
    for (auto lang : languages) {
        auto button = new QRadioButton(Lang2ISO639(lang), this);
        button->setFocusPolicy(Qt::NoFocus);
        layout->addWidget(button);

        if (lang == PopTranslateSettings::instance().activeTargetLanguage()) {
            button->setChecked(true);
        }

        connect(button, &QRadioButton::clicked, [this, lang]() {
            PopTranslateSettings::instance().setActiveTargetLanguage(lang);
            auto text = sourceText();
            clear();
            emit requestTranslate(text);
        });
    }
}

void PopupDialog::setFont(const QFont &font) {
    // setting_.font = font;
    ui->trans_text_edit->setFont(font);
    ui->src_plain_text_edit->setFont(font);

    auto title_font = font;
    title_font.setPointSizeF(font.pointSizeF() * 0.75);
    ui->title_label->setFont(title_font);
}

void PopupDialog::setOpacity(qreal opacity) {
    QPalette pal = palette();
    QColor background_color = pal.color(QWidget::backgroundRole());
    background_color.setAlpha(static_cast<int>(opacity * 255));
    pal.setColor(QPalette::Window, background_color);
    setPalette(pal);

    QPalette pal_edit = ui->trans_text_edit->palette();
    pal_edit.setColor(QPalette::Base, Qt::transparent);
    ui->trans_text_edit->setPalette(pal_edit);
    ui->src_plain_text_edit->setPalette(pal_edit);
}

void PopupDialog::showTranslateResult(
    const AbstractTranslator::Result &result) {
    auto &[title, t_text] = result;

    QFontMetrics metrics(ui->title_label->font());
    if (metrics.horizontalAdvance(title) > ui->title_label->width()) {
        // title is too long, so it is truncated
        QString str =
            QFontMetrics(ui->title_label->font())
                .elidedText(title, Qt::ElideRight, ui->title_label->width());
        ui->title_label->setText(str);
    } else {
        ui->title_label->setText(title);
    }

    // auto plain_text = QTextDocumentFragment::fromHtml(t_text).toPlainText();
    // ui->trans_text_edit->setText(plain_text);
    ui->trans_text_edit->setText(t_text);

    // auto copy translation
    if (PopTranslateSettings::instance().isEnableAutoCopyTranslation()) {
        copyTranslation();
    }
}

// void PopupDialog::mouseMoveEvent(QMouseEvent *event) {
//     showFloatButton(event->pos());
// }

void PopupDialog::leaveEvent(QEvent *event) {
    Q_UNUSED(event);

    if (!isEnableMonitorMode() && !isEnableSrcEditMode() && !cursorInWidget()) {
        this->hide();
    }
}

void PopupDialog::hideEvent(QHideEvent *event) {
    Q_UNUSED(event);
    emit hidden();
    ui->search_lineedit->hide();
    enableSrcEditMode(false);
    hideFloatButton();
}

void PopupDialog::showEvent(QShowEvent *event) {
    Q_UNUSED(event);
    // ui->tools_widget->setMinimumHeight(ui->src_tools_widget->height());
    // ui->bottom_widget->setMinimumHeight(ui->bottom_widget->height());
    enableSrcEditMode(false);
    this->windowHandle()->installEventFilter(this);
    KWindowEffects::enableBlurBehind(
        windowHandle(),
        PopTranslateSettings::instance().isEnableBlur());
}

void PopupDialog::keyPressEvent(QKeyEvent *event) {
    Qt::KeyboardModifiers modifiers = event->modifiers();

    if (QString text = event->text(); ui->search_lineedit->isHidden() &&
                                      !isEnableSrcEditMode() &&
                                      modifiers == Qt::NoModifier &&
                                      !text.isEmpty() && text.at(0).isPrint()) {
        if (!text.at(0).isSpace()) {
            ui->search_lineedit->setText(text);
        }
        ui->search_lineedit->show();
        ui->search_lineedit->setFocus();
        return;
    }

    switch (event->key()) {
        case Qt::Key_Escape:
            if (isEnableSrcEditMode()) {
                enableSrcEditMode(false);
            } else if (ui->search_lineedit->isVisible()) {
                ui->search_lineedit->hide();
                ui->search_lineedit->clear();
                ui->search_lineedit->clearFocus();
            } else {
                this->hide();
            }
            break;
        case Qt::Key_PageUp:
        case Qt::Key_Left:
        case Qt::Key_Home:
            emit showPrevResult();
            break;
        case Qt::Key_PageDown:
        case Qt::Key_Right:
        case Qt::Key_End:
            emit showNextResult();
            break;
        default:
            QWidget::keyPressEvent(event);
    }
}

bool PopupDialog::eventFilter(QObject *filtered, QEvent *event) {
    if (filtered == this->windowHandle() && event->type() == QEvent::Expose) {
        // show window under mouse cursor on wayland
        auto pop_window = qobject_cast<QWindow *>(filtered);
        if (!pop_window->isVisible()) {
            pop_window->setVisible(true);
        }

        auto surface = KWayland::Client::Surface::fromWindow(pop_window);
        auto plasmaSurface = plasmashell_->createSurface(surface, pop_window);

        // this->setWindowFlags(Qt::Window );
        // KWindowSystem::setType(pop_window->winId(), NET::WindowType::Normal);
        // KWindowSystem::setState(pop_window->winId(), NET::KeepAbove);

        // blur window Behind

        bool flag = !isEnableMonitorMode();
        if (flag) {
            plasmaSurface->openUnderCursor();
        }
        plasmaSurface->setSkipTaskbar(flag);
        plasmaSurface->setSkipSwitcher(flag);
        // plasmaSurface->setRole(KWayland::Client::PlasmaShellSurface::Role::Panel);
        // plasmaSurface->setPanelBehavior(KWayland::Client::PlasmaShellSurface::PanelBehavior::AlwaysVisible);
        pop_window->removeEventFilter(this);
    } else if (filtered == ui->trans_text_edit) {
        // Show prev/next button when the mouse is close to the window edge
        if (event->type() == QEvent::HoverMove) {
            auto hover_event = dynamic_cast<QHoverEvent *>(event);
            showFloatButton(hover_event->pos());

        } else if (event->type() == QEvent::Leave) {
            hideFloatButton();
        } else if (event->type() == QEvent::KeyPress) {
            auto key_event = dynamic_cast<QKeyEvent *>(event);
            switch (key_event->key()) {
                case Qt::Key_PageUp:
                case Qt::Key_Left:
                case Qt::Key_Home:
                    emit showPrevResult();
                    break;
                case Qt::Key_PageDown:
                case Qt::Key_Right:
                case Qt::Key_End:
                    emit showNextResult();
                    break;
            }
        }
    }

    return QObject::eventFilter(filtered, event);
}

void PopupDialog::initBottomButtons() {
    ui->bottom_left->layout()->setAlignment(Qt::AlignLeft);
    ui->bottom_right->layout()->setAlignment(Qt::AlignRight);

    connect(ui->copy_button, &QPushButton::released, [this] {
        if (isEnableSrcEditMode()) {
            copySourceText();
        } else {
            copyTranslation();
        }
    });

    connect(ui->pin_button, &QToolButton::clicked, [this](bool checked) {
        bool flag = !isEnableMonitorMode();
        enableMonitorMode(flag);
        PopTranslateSettings::instance().setMonitorClipboard(flag);
        hide();
        show();
    });

    connect(ui->setting_button,
            &QToolButton::released,
            this,
            &PopupDialog::requestShowSettingsWindow);
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

void PopupDialog::initFloatButton() {
    int width = style()->pixelMetric(QStyle::PM_LargeIconSize);

    QSize icon_size(width, width);
    QSize btn_size(width * 2, width * 2);

    btn_prev_->setIconSize(icon_size);
    btn_next_->setIconSize(icon_size);

    btn_prev_->setFixedSize(btn_size);
    btn_next_->setFixedSize(btn_size);

    QString style =
        QString(
            "QPushButton {background-color: palette(light); border: none;"
            "border-radius: %1;}"
            "QPushButton:hover { background-color: palette(Midlight); }"
            "QPushButton:pressed { background-color: palette(Highlight); }")
            .arg(width);

    btn_prev_->setStyleSheet(style);
    btn_next_->setStyleSheet(style);

    hideFloatButton();

    QHBoxLayout *layout = new QHBoxLayout(ui->trans_text_edit);
    layout->addWidget(btn_prev_);
    layout->addWidget(btn_next_);
    layout->setAlignment(btn_prev_, Qt::AlignLeft);
    layout->setAlignment(btn_next_, Qt::AlignRight);

    // If mouse tracking is switched on, mouse move events occur even if no
    // mouse button is pressed
    // setMouseTracking(true);

    ui->trans_text_edit->installEventFilter(this);

    connect(btn_prev_,
            &QPushButton::clicked,
            this,
            &PopupDialog::showPrevResult);

    connect(btn_next_,
            &QPushButton::clicked,
            this,
            &PopupDialog::showNextResult);
}

void PopupDialog::showFloatButton(QPoint cursor_pos) {
    // TODO: hide float button when the mouse button is pressed

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
        hideFloatButton();
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
    hlayout->setStretchFactor(ui->title_label, 4);
    hlayout->setStretchFactor(ui->pronounce_button, 1);

    // connect(btn_prev_, &QPushButton::clicked, [this] {
    //     indicator_->prevPage();
    // });

    // connect(btn_next_, &QPushButton::clicked, [this] {
    //     indicator_->nextPage();
    // });

    connect(this, &PopupDialog::newResultsAvailable, [this](int index) {
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

    // reinstall event filter in animation_group2_ finished signal
    connect(animation_group2_, &QAnimationGroup::finished, [this] {
        ui->trans_text_edit->installEventFilter(this);
        showFloatButton(QCursor::pos());
    });
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
    // When the animation starts, uninstall the event filter.
    // Will reinstall event filter in animation_group2_ finished signal
    ui->trans_text_edit->removeEventFilter(this);

    hideFloatButton();

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
    // When the animation starts, uninstall the event filter.
    // Will reinstall event filter in animation_group2_ finished signal
    ui->trans_text_edit->removeEventFilter(this);

    hideFloatButton();

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
                                     &PopupDialog::newResultsAvailable,
                                     resultAvailable);

    resultAvailable->addTransition(this,
                                   &PopupDialog::showPrevResult,
                                   requestPrevResult);

    resultAvailable->addTransition(this,
                                   &PopupDialog::showNextResult,
                                   requestNextResult);

    requestNextResult->addTransition(animation_group1_,
                                     &QAbstractAnimation::finished,
                                     readyShowResult);

    requestNextResult->addTransition(this,
                                     &PopupDialog::noNextResult,
                                     resultAvailable);

    requestPrevResult->addTransition(animation_group1_,
                                     &QAbstractAnimation::finished,
                                     readyShowResult);

    requestPrevResult->addTransition(this,
                                     &PopupDialog::noPrevResult,
                                     resultAvailable);

    readyShowResult->addTransition(animation_group2_,
                                   &QAbstractAnimation::finished,
                                   resultAvailable);

    readyShowResult->addTransition(this,
                                   &PopupDialog::cleared,
                                   resultUnavailable);

    connect(requestNextResult, &QState::entered, [this] {
        if (hasNextResult()) {
            result_index_++;
            startAnimationNext();
            indicator_->nextPage();
        } else {
            emit noNextResult();
        }
    });

    connect(requestPrevResult, &QState::entered, [this] {
        if (hasPrevResult()) {
            result_index_--;
            startAnimationPrev();
            indicator_->prevPage();
        } else {
            emit noPrevResult();
        }
    });

    connect(readyShowResult, &QState::entered, [this] {
        showTranslateResult(translate_results_.at(result_index_));
    });

    result_state_machine_.start();
}

void PopupDialog::clear() {
    ui->title_label->setText("PopTranslate");
    ui->trans_text_edit->clear();
    ui->src_plain_text_edit->clear();

    translate_results_.clear();
    result_index_ = -1;
    indicator_->clear();

    emit cleared();
}

void PopupDialog::enableSrcEditMode(bool enable) {
    ui->src_plain_text_edit->setVisible(enable);
    ui->src_tools_widget->setVisible(enable);

    ui->trans_text_edit->setVisible(!enable);
    ui->tools_widget->setVisible(!enable);

    if (enable) {
        ui->search_lineedit->hide();
        ui->src_plain_text_edit->setFocus();
        ui->src_plain_text_edit->moveCursor(QTextCursor::End);
        ui->show_src_toolbutton->setIcon(QIcon::fromTheme("arrow-down-double"));
        ui->show_src_toolbutton->setToolTip(tr("Hide Source Text"));
        ui->copy_button->setToolTip(tr("Copy Source Text"));
    } else {
        // ui->trans_text_edit->setFocus();
        ui->title_label->setFocus();
        ui->src_plain_text_edit->clearFocus();
        ui->show_src_toolbutton->setIcon(QIcon::fromTheme("arrow-up-double"));
        ui->show_src_toolbutton->setToolTip(tr("Show Source Text"));
        ui->copy_button->setToolTip(tr("Copy Translation"));
    }
}

void PopupDialog::loadSettings() {
    auto &settings = PopTranslateSettings::instance();

    setTargetLanguages(settings.targetLanguages());
    setFont(settings.font());
    setOpacity(settings.opacity());

    connect(&settings,
            &PopTranslateSettings::targetLanguagesChanged,
            this,
            &PopupDialog::setTargetLanguages);

    connect(&settings,
            &PopTranslateSettings::fontChanged,
            this,
            &PopupDialog::setFont);

    connect(&settings,
            &PopTranslateSettings::opacityChanged,
            this,
            &PopupDialog::setOpacity);

    resize(settings.popupWindowSize());
}