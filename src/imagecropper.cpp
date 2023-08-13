#include "imagecropper.h"

#include <QDebug>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>

ImageCropper::ImageCropper(QLabel *parent) : QLabel(parent), dragging_(false) {
    setWindowFlags(Qt::FramelessWindowHint);
    setWindowState(Qt::WindowFullScreen);
}

void ImageCropper::cropImage(const QPixmap &img) {
    setPixmap(img);
    show();
}

void ImageCropper::mousePressEvent(QMouseEvent *event) {
    start_ = event->pos();
    dragging_ = true;
}

void ImageCropper::mouseMoveEvent(QMouseEvent *event) {
    if (dragging_) {
        end_ = event->pos();
        this->update();
    }
}

void ImageCropper::mouseReleaseEvent(QMouseEvent *event) {
    this->hide();
    QPixmap pix = pixmap(Qt::ReturnByValue).copy(getRect());
    emit imageCropped(pix);
    dragging_ = false;
    start_ = end_ = QPoint();
}

void ImageCropper::paintEvent(QPaintEvent *event) {
    QLabel::paintEvent(event);

    QPainterPath border;
    border.setFillRule(Qt::WindingFill);
    border.addRect(this->rect());

    QPainterPath crop_area;
    crop_area.setFillRule(Qt::WindingFill);
    crop_area.addRect(getRect());

    QPainterPath end_path = border.subtracted(crop_area);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillPath(end_path, QColor(0, 0, 0, 100));
}

QRect ImageCropper::getRect() {
    int x = start_.x() < end_.x() ? start_.x() : end_.x();
    int y = start_.y() < end_.y() ? start_.y() : end_.y();
    int width = abs(start_.x() - end_.x());
    int height = abs(start_.y() - end_.y());
    return QRect(x, y, width, height);
}
