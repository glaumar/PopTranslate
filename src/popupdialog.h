#pragma once

#include <KWayland/Client/blur.h>
#include <KWayland/Client/plasmashell.h>

#include <QClipboard>
#include <QDebug>
#include <QMenu>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QSequentialAnimationGroup>
#include <QStateMachine>
#include <QTextDocumentFragment>
#include <QVector>

#include "abstracttranslator.h"
#include "pageindicator.h"
#include "poptranslatesettings.h"
#include "tts.h"
#include "ui_popupdialog.h"

namespace Ui {
class PopupDialog;
}

class PopupDialog : public QWidget {
    Q_OBJECT

   public:
    explicit PopupDialog(QWidget *parent = nullptr);
    ~PopupDialog();

    void enableMonitorMode(bool enable);
    void addTranslateResult(const AbstractTranslator::Result &result);
    void setTargetLanguages(QVector<QOnlineTranslator::Language> languages);
    void setFont(const QFont &font);
    void setOpacity(qreal opacity);
    void clear();
    void enableSrcEditMode(bool enable);

    inline bool isEnableSrcEditMode() const {
        return ui->src_plain_text_edit->isVisible();
    }

    inline QString sourceText() const {
        return ui->src_plain_text_edit->toPlainText();
    }

    inline QString translationText() const {
        return ui->trans_text_edit->toPlainText();
    }

    inline void setSourceText(const QString &text) {
        ui->src_plain_text_edit->setPlainText(text);
    }

   signals:
    void requestTranslate(const QString &text);
    void requestSpeak(
        const QString &text,
        QOnlineTranslator::Language lang = QOnlineTranslator::Auto);
    void requestShowSettingsWindow();
    void newResultsAvailable(int index);
    void cleared();
    void hidden();
    void showNextResult();
    void showPrevResult();
    void noNextResult();
    void noPrevResult();

   protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *filtered, QEvent *event) override;

   private:
    inline void copySourceText() const {
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(sourceText());
    }

    inline void copyTranslation() const {
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(translationText());
    }

    void showTranslateResult(const AbstractTranslator::Result &result);

    inline bool hasNextResult() const {
        return result_index_ >= 0 &&
               result_index_ < translate_results_.size() - 1;
    }
    inline bool hasPrevResult() const {
        return result_index_ > 0 &&
               result_index_ <= translate_results_.size() - 1;
    }

    inline bool isEnableMonitorMode() const {
        return PopTranslateSettings::instance().monitorClipboard();
    }

    inline bool cursorInWidget() const {
        auto pos = QCursor::pos();
        uint deviation = 10;
        return pos.x() > x() + deviation && pos.x() <= width() - deviation &&
               pos.y() > y() + deviation && pos.y() <= height() - deviation;
    }

    void loadSettings();
    void initBottomButtons();
    void initWaylandConnection();
    void initFloatButton();
    void showFloatButton(QPoint cursor_pos);
    void initPageIndicator();
    void initAnimation();
    void setEffectAnimation(QWidget *widget);
    void startAnimationPrev();
    void startAnimationNext();
    void initStateMachine();

    KWayland::Client::PlasmaShell *plasmashell_;
    Ui::PopupDialog *ui;
    QVector<AbstractTranslator::Result> translate_results_;
    int result_index_;
    QPushButton *btn_prev_;
    QPushButton *btn_next_;
    PageIndicator *indicator_;
    QPropertyAnimation *animation_title_1_;
    QPropertyAnimation *animation_title_2_;
    QPropertyAnimation *animation_translation_1_;
    QPropertyAnimation *animation_translation_2_;
    QParallelAnimationGroup *animation_group1_;
    QParallelAnimationGroup *animation_group2_;
    QSequentialAnimationGroup *animation_group_all_;
    int animation_duration_;
    QStateMachine result_state_machine_;
};
