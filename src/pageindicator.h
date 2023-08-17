#pragma once

#include <QLabel>
#include <QPixmap>
#include <QWidget>

namespace Ui {
class PageIndicator;
}

class PageIndicator : public QWidget {
    Q_OBJECT
   public:
    explicit PageIndicator(QWidget *parent = nullptr);

    inline void setActivePixmap(const QPixmap &pixmap) {
        pixmap_active_ = pixmap;
    }

    inline void setInActivePixmap(const QPixmap &pixmap) {
        pixmap_inactive_ = pixmap;
    }

    void initPages(int total, int current = 1);
    void addPages(int n = 1);
    void nextPage();
    void prevPage();
    void clear();

   private:
    QPixmap defaultActivePixmap(qreal ratio = 1) const;
    QPixmap defaultInActivePixmap(qreal ratio = 1) const;

    Ui::PageIndicator *ui;
    QPixmap pixmap_active_;
    QPixmap pixmap_inactive_;
    QVector<QLabel *> labels_;
    int index_;
};
