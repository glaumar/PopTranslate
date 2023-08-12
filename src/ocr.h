// This file is extracted and modified from the Crow Translate project for OCR
// recognition purposes.

/*
 *  Copyright © 2018-2023 Hennadii Chernyshchyk <genaloner@gmail.com>
 *
 *  This file is part of Crow Translate.
 *
 *  Crow Translate is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Crow Translate is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Crow Translate. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <tesseract/baseapi.h>
#include <tesseract/ocrclass.h>

#include <QFuture>
#include <QObject>

class QDir;

class Ocr : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(Ocr)

   public:
    explicit Ocr(QObject *parent = nullptr);

    void setConvertLineBreaks(bool convert);

    QStringList availableLanguages() const;
    QByteArray languagesString() const;
    bool init(const QByteArray &languages,
              const QByteArray &languagesPath,
              const QMap<QString, QVariant> &parameters);

    void recognize(const QPixmap &pixmap);
    void cancel();

    static QStringList availableLanguages(const QString &languagesPath);

   public slots:
    void applyParameters(const QMap<QString, QVariant> &parameters);

   signals:
    void recognized(const QString &text);
    void canceled();

   private:
    static QStringList parseLanguageFiles(const QDir &directory);

    QMap<QString, QVariant> m_parameters;
    QFuture<void> m_future;

    tesseract::TessBaseAPI m_tesseract;
    tesseract::ETEXT_DESC m_monitor;

    bool m_convertLineBreaks = false;
};
