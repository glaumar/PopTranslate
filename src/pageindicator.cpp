#include "pageindicator.h"

#include <QDebug>
#include <QPainter>
#include <QPropertyAnimation>

#include "ui_pageindicator.h"

PageIndicator::PageIndicator(QWidget *parent)
    : QWidget(parent), ui(new Ui::PageIndicator()), index_(-1), labels_() {
    ui->setupUi(this);

    pixmap_active_ = defaultActivePixmap();
    pixmap_inactive_ = defaultInActivePixmap();
}

QPixmap PageIndicator::defaultInActivePixmap(qreal ratio) const {
    int base = static_cast<int>(8 * ratio);

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
        auto label = new QLabel(this);
        label->setPixmap(pixmap_inactive_);
        ui->horizontalLayout->addWidget(label);
        labels_.append(label);
    }
}

void PageIndicator::nextPage() {
    if (labels_.isEmpty()) {
        return;
    }

    int next = index_ + 1;

    if (next < 0) {
        next = 0;
    } else if (next > labels_.size() - 1) {
        next = labels_.size() - 1;
    }

    if (index_ >= 0 && index_ < labels_.size()) {
        labels_[index_]->setPixmap(pixmap_inactive_);
    }

    index_ = next;
    labels_[next]->setPixmap(pixmap_active_);
}

void PageIndicator::prevPage() {
    if (labels_.isEmpty()) {
        return;
    }

    int next = index_ - 1;

    if (next < 0) {
        next = 0;
    } else if (next > labels_.size() - 1) {
        next = labels_.size() - 1;
    }

    if (index_ >= 0 && index_ < labels_.size()) {
        labels_[index_]->setPixmap(pixmap_inactive_);
    }

    index_ = next;
    labels_[next]->setPixmap(pixmap_active_);
}

void PageIndicator::clear() {
    for (auto label : labels_) {
        ui->horizontalLayout->removeWidget(label);
        delete label;
    }
    labels_.clear();
    index_ = -1;
}

QPixmap PageIndicator::defaultActivePixmap(qreal ratio) const {
    int base = static_cast<int>(8 * ratio);

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