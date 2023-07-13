#ifndef POPUPDIALOG_H
#define POPUPDIALOG_H

#include <KWayland/Client/plasmashell.h>

#include <QDebug>
#include <QDialog>
#include <QMenu>

#include "defaultsettings.h"
#include "qonlinetranslator.h"

namespace Ui {
class PopupDialog;
}

class PopupDialog : public QDialog {
    Q_OBJECT

   public:
    explicit PopupDialog(QWidget *parent = nullptr);
    ~PopupDialog();
    bool event(QEvent *event) override;
    bool isNormalWindow() const;
    void setNormalWindow(bool on = false);
   public slots:
    void translate(const QString &text);
    void retranslate();
    void setTranslateEngine(QOnlineTranslator::Engine engine);
    void setTargetLanguages(QVector<QOnlineTranslator::Language> languages);
    void setFont(const QFont &font);

   private:
    bool eventFilter(QObject *filtered, QEvent *event) override;
    void initContextMenu();

    bool flag_normal_window_;
    KWayland::Client::PlasmaShell *plasmashell_;
    Ui::PopupDialog *ui;
    QOnlineTranslator translator_;
    const DefaultSettings default_;
    QOnlineTranslator::Engine current_translate_engine_;
    QOnlineTranslator::Language current_target_language_;
    QMenu context_menu_;
    QMenu engine_menu_;
};

#endif  // POPUPDIALOG_H
