#pragma once
#include <tesseract/baseapi.h>

#include <QImage>
#include <QObject>

class Ocr : public QObject{
    Q_OBJECT
   public:
    explicit Ocr(QObject *parent = nullptr);
    ~Ocr();
    void recognize(const QImage &image);

   signals:
    void recognized(const QString &text);

   private:
    tesseract::TessBaseAPI api_;
};
