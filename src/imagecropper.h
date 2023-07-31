#pragma once

#include <QLabel>

class ImageCropper : public QLabel {
    Q_OBJECT
   public:
    explicit ImageCropper(QLabel *parent = nullptr);

   public slots:
    void cropImage(const QPixmap &img);

   signals:
    void imageCropped(QPixmap cropped_img);

   protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

   private:
    QRect crop_rect_;
    bool dragging_;
};