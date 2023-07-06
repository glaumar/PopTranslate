#ifndef POPUPDIALOG_H
#define POPUPDIALOG_H

#include <KWayland/Client/plasmashell.h>
#include <QDialog>
#include <QDebug>
#include "qonlinetranslator.h"

namespace Ui {
class PopupDialog;
}

class PopupDialog : public QDialog {
    Q_OBJECT

   public:
    explicit PopupDialog(QWidget *parent = nullptr);
    ~PopupDialog();
    void SetTransWords(const QString &words);
    bool event(QEvent *event) override;
    bool isNormalWindow() const;
    void setNormalWindow(bool on = false);
private:
    bool eventFilter(QObject *filtered, QEvent *event) override;

    bool flag_normal_window;
    KWayland::Client::PlasmaShell *plasmashell;
    Ui::PopupDialog *ui;
    QOnlineTranslator translator_;
};

#endif  // POPUPDIALOG_H
