#ifndef POPUPDIALOG_H
#define POPUPDIALOG_H

#include <KWayland/Client/blur.h>
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
    void setNormalWindow(bool enable = false);
   public slots:
    void translate(const QString &text);
    void retranslate();
    void setTranslateEngine(QOnlineTranslator::Engine engine);
    void setTargetLanguages(QVector<QOnlineTranslator::Language> languages);
    void setFont(const QFont &font);
    void setOpacity(qreal opacity);
    void enableBlur(bool enable);

   private:
    bool eventFilter(QObject *filtered, QEvent *event) override;
    void initContextMenu();
    void initWaylandConnection();
    void initTranslator();

    bool flag_normal_window_;
    KWayland::Client::PlasmaShell *plasmashell_;
    Ui::PopupDialog *ui;
    QOnlineTranslator translator_;
    DefaultSettings setting_;
    QMenu context_menu_;
    QMenu engine_menu_;
};

#endif  // POPUPDIALOG_H
