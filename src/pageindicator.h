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
    ~PageIndicator();

    inline void setActivePixmap(const QPixmap &pixmap) {
        pixmap_active_ = pixmap;
    }

    inline void setInActivePixmap(const QPixmap &pixmap) {
        pixmap_inactive_ = pixmap;
    }

    inline void setAnimationDuration(int duration) {
        animation_duration_ = duration;
    }

    void initPages(int total, int current = 1);
    void addPages(int n = 1);
    void nextPage();
    void prevPage();
    void clear();
    void startAnimationNext(QSize active_size, QSize inactive_size);
    void startAnimationPrev(QSize active_size, QSize inactive_size);

   private:
    QPixmap defaultActivePixmap(qreal ratio = 1) const;
    QPixmap defaultInActivePixmap(qreal ratio = 1) const;

    Ui::PageIndicator *ui;
    QPixmap pixmap_active_;
    QPixmap pixmap_inactive_;
    QVector<QLabel *> labels_;
    int index_;
    int animation_duration_;
};
