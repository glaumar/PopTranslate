#include "pageindicator.h"

#include <QDebug>
#include <QPainter>
#include <QPropertyAnimation>
#include <QStyle>

#include "ui_pageindicator.h"

PageIndicator::PageIndicator(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::PageIndicator()),
      index_(-1),
      labels_(),
      animation_duration_(150) {
    ui->setupUi(this);

    pixmap_active_ = defaultActivePixmap();
    pixmap_inactive_ = defaultInActivePixmap();
}

QPixmap PageIndicator::defaultInActivePixmap(qreal ratio) const {
    auto width = style()->pixelMetric(QStyle::PM_SmallIconSize) / 2;
    int base = static_cast<int>(width * ratio);

    QPixmap pix(base, base);
    pix.fill(Qt::transparent);

    auto color = palette().color(QPalette::WindowText);
    QPainter painter(&pix);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setBrush(color);
    painter.drawEllipse(0, 0, base, base);
    painter.end();
    return pix;
}

QPixmap PageIndicator::defaultActivePixmap(qreal ratio) const {
    int width = style()->pixelMetric(QStyle::PM_SmallIconSize) / 2;
    qDebug() << width;

    int base = static_cast<int>(width * ratio);

    QPixmap pix(base * 3, base);
    pix.fill(Qt::transparent);

    auto color = palette().color(QPalette::WindowText);
    QPainter painter(&pix);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setBrush(color);
    painter.drawRoundedRect(0, 0, base * 3, base, base / 2, base / 2);
    painter.end();
    return pix;
}

void PageIndicator::initPages(int total, int current) {
    if (total < 1) {
        return;
    }

    if (current < 1) {
        current = 1;
    }

    if (current > total) {
        current = total;
    }
    addPages(total);

    labels_[current - 1]->setPixmap(pixmap_active_);
    index_ = current - 1;
}

void PageIndicator::addPages(int n) {
    for (int i = 0; i < n; ++i) {
        auto label = new QLabel(nullptr);
        label->setPixmap(pixmap_inactive_);
        ui->horizontalLayout->addWidget(label);
        labels_.append(label);
    }
}

void PageIndicator::nextPage() {
    if (labels_.isEmpty() || index_ >= labels_.size() - 1) {
        return;
    }

    if (index_ < 0) {
        index_ = 0;
        labels_[0]->setPixmap(pixmap_active_);
        return;
    }

    QSize active_label_size = labels_[index_]->size();
    QSize inactive_label_size = labels_[index_ + 1]->size();

    labels_[index_]->setPixmap(pixmap_inactive_);
    index_++;
    labels_[index_]->setPixmap(pixmap_active_);

    startAnimationNext(active_label_size, inactive_label_size);
}

void PageIndicator::prevPage() {
    if (labels_.isEmpty() || index_ <= 0) {
        return;
    }

    if (index_ > labels_.size() - 1) {
        index_ = labels_.size() - 1;
        labels_[index_]->setPixmap(pixmap_active_);
        return;
    }

    QSize active_label_size = labels_[index_]->size();
    QSize inactive_label_size = labels_[index_ - 1]->size();

    labels_[index_]->setPixmap(pixmap_inactive_);
    index_--;
    labels_[index_]->setPixmap(pixmap_active_);

    startAnimationPrev(active_label_size, inactive_label_size);
}

void PageIndicator::clear() {
    for (auto label : labels_) {
        ui->horizontalLayout->removeWidget(label);
        delete label;
    }
    labels_.clear();
    index_ = -1;
}

void PageIndicator::startAnimationNext(QSize active_size, QSize inactive_size) {
    auto top_left = labels_[index_]->pos();
    top_left.setX(top_left.x() - active_size.width() + inactive_size.width());

    QRect active = QRect(top_left, active_size);
    QRect inactive = QRect(top_left, inactive_size);

    auto *animation = new QPropertyAnimation(labels_[index_], "geometry");
    animation->setDuration(animation_duration_);
    animation->setStartValue(inactive);
    animation->setEndValue(active);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void PageIndicator::startAnimationPrev(QSize active_size, QSize inactive_size) {
    auto top_left_ed = labels_[index_]->pos();
    auto top_left_st = top_left_ed;
    top_left_st.setX(top_left_ed.x() + active_size.width() -
                     inactive_size.width());

    QRect active = QRect(top_left_ed, active_size);
    QRect inactive = QRect(top_left_st, inactive_size);

    auto *animation = new QPropertyAnimation(labels_[index_], "geometry");
    animation->setDuration(animation_duration_);
    animation->setStartValue(inactive);
    animation->setEndValue(active);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}
