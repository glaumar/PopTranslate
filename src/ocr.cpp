#include "ocr.h"

#include <QDebug>

Ocr::Ocr(QObject *parent) : QObject(parent) { api_.Init(nullptr, "eng"); }

Ocr::~Ocr() { api_.End(); }

void Ocr::recognize(const QImage &image) {
    api_.SetImage(image.bits(),
                  image.width(),
                  image.height(),
                  4,
                  image.bytesPerLine());

    if (api_.Recognize(nullptr)) {
        qWarning() << tr("OCR: Failed to recognize text.");
    } else {
        emit recognized(api_.GetUTF8Text());
    }
}