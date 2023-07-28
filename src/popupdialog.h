#ifndef POPUPDIALOG_H
#define POPUPDIALOG_H

#include <KWayland/Client/blur.h>
#include <KWayland/Client/plasmashell.h>

#include <QDebug>
#include <QDialog>
#include <QList>
#include <QMenu>

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
    void removeDictionaries(const QStringList &dicts);

   signals:
    void settingsActionTriggered();
    void translateResultsAvailable(int index);

   private:
    bool eventFilter(QObject *filtered, QEvent *event) override;
    void initContextMenu();
    void initWaylandConnection();
    void initTranslator();
    void initDictionaries();

    bool flag_normal_window_;
    KWayland::Client::PlasmaShell *plasmashell_;
    Ui::PopupDialog *ui;
    QOnlineTranslator translator_;
    DefaultSettings setting_;
    QMenu context_menu_;
    QMenu engine_menu_;
    QAction *action_source_text_;
    Dictionaries dicts_;
    QList<QString> translate_results_;
    QList<QString>::const_iterator current_translate_result_;
};

#endif  // POPUPDIALOG_H
