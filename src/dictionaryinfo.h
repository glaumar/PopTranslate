#pragma once

#include <QDataStream>
#include <QMetaType>
#include <QString>
#include <QVector>

#include "qonlinetranslator.h"

struct DictionaryInfo {
    QString filename;
    QOnlineTranslator::Language target_language;

    friend QDataStream &operator<<(QDataStream &arch,
                                   const DictionaryInfo &object) {
        arch << object.filename;
        arch << object.target_language;
        return arch;
    }

    friend QDataStream &operator>>(QDataStream &arch, DictionaryInfo &object) {
        arch >> object.filename;
        arch >> object.target_language;
        return arch;
    }

    friend uint qHash(const DictionaryInfo &key, uint seed = 0) {
        return qHash(key.filename, seed);
    }

    bool operator==(const DictionaryInfo &other) const {
        return filename == other.filename;
    }

    bool operator!=(const DictionaryInfo &other) const {
        return !(*this == other);
    }

    bool operator<(const DictionaryInfo &other) const {
        return filename < other.filename;
    }

    bool operator>(const DictionaryInfo &other) const {
        return filename > other.filename;
    }
};

Q_DECLARE_METATYPE(DictionaryInfo)