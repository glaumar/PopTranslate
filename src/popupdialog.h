#pragma once

#include <KWayland/Client/blur.h>
#include <KWayland/Client/plasmashell.h>

#include <QClipboard>
#include <QDebug>
#include <QMenu>
#include <QPair>
#include <QTextDocumentFragment>
#include <QVector>

#include "defaultsettings.h"
#include "dictionaries.h"
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
    bool event(QEvent *event) override;
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
    // void removeDictionaries(const QStringList &dicts);
    void enableAutoCopyTranslation(bool enable);
    void showTranslateResult(const QPair<QString, QString> &result);

    inline QString sourceText() const {
        return ui->src_plain_text_edit->toPlainText();
    }

    inline QString translationText() const {
        return ui->trans_text_edit->toPlainText();
    }

   signals:
    void settingsActionTriggered();
    void translateResultsAvailable(int index);

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
    bool eventFilter(QObject *filtered, QEvent *event) override;
    void initContextMenu();
    void initWaylandConnection();
    void initTranslator();
    void initDictionaries();
    void initToolbar();

    bool flag_normal_window_;
    KWayland::Client::PlasmaShell *plasmashell_;
    Ui::PopupDialog *ui;
    QOnlineTranslator translator_;
    DefaultSettings setting_;
    QMenu context_menu_;
    QMenu engine_menu_;
    QAction *action_source_text_;
    Dictionaries dicts_;
    QVector<QPair<QString, QString>> translate_results_;
    int result_index_;
};
