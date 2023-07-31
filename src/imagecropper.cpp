#include "imagecropper.h"

#include <QDebug>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

ImageCropper::ImageCropper(QLabel *parent)
    : QLabel(parent), crop_rect_(), dragging_(false) {
    setWindowFlags(Qt::FramelessWindowHint);
    setWindowState(Qt::WindowFullScreen);
}

void ImageCropper::cropImage(const QPixmap &img) {
    setPixmap(img);
    show();
}

void ImageCropper::mousePressEvent(QMouseEvent *event) {
    crop_rect_.setTopLeft(event->pos());
    dragging_ = true;
}

void ImageCropper::mouseMoveEvent(QMouseEvent *event) {
    if (dragging_) {
        crop_rect_.setBottomRight(event->pos());
        this->update();
    }
}

void ImageCropper::mouseReleaseEvent(QMouseEvent *event) {
    this->hide();
    emit imageCropped(pixmap()->copy(crop_rect_));

    dragging_ = false;
    crop_rect_ = QRect();
}

void ImageCropper::paintEvent(QPaintEvent *event) {
    QLabel::paintEvent(event);

    QPainterPath border;
    border.setFillRule(Qt::WindingFill);
    border.addRect(this->rect());

    QPainterPath crop_area;
    crop_area.setFillRule(Qt::WindingFill);
    crop_area.addRect(crop_rect_);

    QPainterPath end_path = border.subtracted(crop_area);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillPath(end_path, QColor(0, 0, 0, 100));
}