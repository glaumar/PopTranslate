#pragma once

#include <KWayland/Client/blur.h>
#include <KWayland/Client/plasmashell.h>

#include <QDebug>
#include <QDialog>
#include <QVector>
#include <QMenu>
#include <QPair>

#include "defaultsettings.h"
#include "dictionaries.h"
#include "qonlinetranslator.h"
#include "ui_popupdialog.h"
namespace Ui {
class PopupDialog;
}

class PopupDialog : public QDialog {
    Q_OBJECT

   public:
    explicit PopupDialog(QWidget *parent = nullptr);
    ~PopupDialog();
    bool event(QEvent *event) override;
    void setNormalWindow(bool enable = false);

    inline bool isNormalWindow() const { return flag_normal_window_; }
    inline bool isSrcTextEditVisible() const {
        return action_source_text_->isChecked();
    }

    inline void setSrcTextEditVisible(bool visible) {
        ui->src_plain_text_edit->setVisible(visible);
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
    // void removeDictionaries(const QStringList &dicts);

   signals:
    void settingsActionTriggered();
    void translateResultsAvailable(int index);

   private:
    bool eventFilter(QObject *filtered, QEvent *event) override;
    void initContextMenu();
    void initWaylandConnection();
    void initTranslator();
    void initDictionaries();
    void initToolbar();

    inline void showTranslateResult(const QPair<QString,QString>& result)
    {
            auto &[s_text, t_text] = result;
            ui->title_label->setText(s_text);
            ui->trans_text_edit->setText(t_text);
    }

    bool flag_normal_window_;
    KWayland::Client::PlasmaShell *plasmashell_;
    Ui::PopupDialog *ui;
    QOnlineTranslator translator_;
    DefaultSettings setting_;
    QMenu context_menu_;
    QMenu engine_menu_;
    QAction *action_source_text_;
    Dictionaries dicts_;
    QVector<QPair<QString,QString>> translate_results_;
    int result_index_;
};
