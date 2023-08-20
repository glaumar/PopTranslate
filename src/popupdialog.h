#pragma once

#include <KWayland/Client/blur.h>
#include <KWayland/Client/plasmashell.h>

#include <QClipboard>
#include <QDebug>
#include <QMediaPlayer>
#include <QMenu>
#include <QPair>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QSequentialAnimationGroup>
#include <QStateMachine>
#include <QTextDocumentFragment>
#include <QVector>

#include "defaultsettings.h"
#include "dictionaries.h"
#include "pageindicator.h"
#include "qonlinetranslator.h"
#include "ui_popupdialog.h"

namespace Ui {
class PopupDialog;
}

class PopupDialog : public QWidget {
    Q_OBJECT

   public:
    explicit PopupDialog(QWidget *parent = nullptr);
    ~PopupDialog();
    void setNormalWindow(bool enable = false);

    inline bool isNormalWindow() const { return flag_normal_window_; }
    inline bool isSrcTextEditVisible() const {
        return action_source_text_->isChecked();
    }

    inline void setSrcTextEditVisible(bool visible) {
        ui->src_plain_text_edit->setVisible(visible);
        ui->horizon_line->setVisible(visible);
        action_source_text_->setChecked(visible);
    }
   public slots:
    void translate(const QString &text);
    void retranslate();
    void setTranslateEngine(QOnlineTranslator::Engine engine);
    void setTargetLanguages(QVector<QOnlineTranslator::Language> languages);
    void setFont(const QFont &font);
    void setOpacity(qreal opacity);
    void enableBlur(bool enable);
    void setDictionaries(const QStringList &dicts);
    void enableAutoCopyTranslation(bool enable);
    void showTranslateResult(const QPair<QString, QString> &result);

    inline bool hasNextResult() const {
        return result_index_ >= 0 &&
               result_index_ < translate_results_.size() - 1;
    }
    inline bool hasPrevResult() const { return result_index_ > 0; }

    inline QString sourceText() const {
        return ui->src_plain_text_edit->toPlainText();
    }

    inline QString translationText() const {
        return ui->trans_text_edit->toPlainText();
    }

   signals:
    void settingsActionTriggered();
    void translateResultsAvailable(int index);
    void cleared();

   protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    bool event(QEvent *event) override;
    bool eventFilter(QObject *filtered, QEvent *event) override;

   private slots:
    inline void copySourceText() const {
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(sourceText());
    }

    inline void copyTranslation() const {
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(translationText());
    }

   private:
    void initContextMenu();
    void initWaylandConnection();
    void initTranslator();
    void initDictionaries();
    void initFloatButton();
    void showFloatButton(QPoint cursor_pos);
    void initPageIndicator();
    void initAnimation();
    void setEffectAnimation(QWidget *widget);
    void startAnimationPrev();
    void startAnimationNext();
    void initStateMachine();
    void initTts();
    void speakText(const QString &text, QOnlineTranslator::Language lang);
    void prepareTextAudio(const QString &text, QOnlineTranslator::Language lang);
    void clear();

    bool flag_normal_window_;
    KWayland::Client::PlasmaShell *plasmashell_;
    Ui::PopupDialog *ui;
    QOnlineTranslator translator_;
    QMediaPlayer *player_;
    DefaultSettings setting_;
    QMenu context_menu_;
    QMenu engine_menu_;
    QAction *action_source_text_;
    Dictionaries dicts_;
    QVector<QPair<QString, QString>> translate_results_;
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
    bool speak_after_translate_;
};
