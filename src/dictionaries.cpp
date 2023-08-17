#include "dictionaries.h"

#include <QDebug>
#include <QFileInfo>

// macro “F” definition conflicts
#undef F
#include <QtConcurrent>

Dictionaries::Dictionaries() {}
Dictionaries::~Dictionaries() {}

void Dictionaries::clear() {
    dicts_.clear();
    dict_names_.clear();
}

void Dictionaries::addDict(const QString& filename) {
    if (dicts_.contains(filename)) {
        qDebug() << tr("dictionary already exists: %1").arg(filename);
        return;
    }

    if (!fileCheck(filename)) {
        return;
    }

    qDebug() << tr("addDict: %1").arg(filename);
    QSharedPointer<mdict::Mdict> md_p(new mdict::Mdict(filename.toStdString()));
    md_p->init();
    dicts_.insert(filename, md_p);
    dict_names_.append(filename);
}

void Dictionaries::addDicts(const QStringList& filenames) {
    for (auto& filename : filenames) {
        addDict(filename);
    }
}

void Dictionaries::setDicts(const QStringList& filenames) {
    QSet<QString> old_set(dict_names_.begin(), dict_names_.end());
    QSet<QString> new_set(filenames.begin(), filenames.end());
    auto diff = old_set - new_set;

    removeDicts(diff.values());
    addDicts(filenames);

    dict_names_ = filenames;
    QMutableStringListIterator it(dict_names_);
    while (it.hasNext()) {
        if (!dicts_.contains(it.next())) {
            it.remove();
        }
    };
}

void Dictionaries::removeDict(const QString& filename) {
    if (!dicts_.contains(filename)) {
        return;
    }

    dicts_.remove(filename);
    for (int i = 0; i < dict_names_.size(); i++) {
        if (dict_names_[i] == filename) {
            dict_names_.removeAt(i);
            qDebug() << tr("removeDict: %1").arg(filename);
            break;
        }
    }
}
void Dictionaries::removeDicts(const QStringList& filenames) {
    for (auto& filename : filenames) {
        removeDict(filename);
    }
}

QVector<QPair<QString, QString>> Dictionaries::lookup(const QString& word) {
    qDebug() << "lookup:" << word;
    QVector<QPair<QString, QString>> results;
    for (auto& dict_name : dict_names_) {
        qDebug() << "dict_name:" << dict_name;
        auto dict = dicts_.value(dict_name);
        auto result = dict->lookup(word.toStdString());
        if (result != "") {
            qDebug() << "found:" << QString::fromStdString(result);
            auto dict_basename = QFileInfo(dict_name).baseName();
            QPair<QString, QString> result_pair(dict_basename,
                                                QString::fromStdString(result));
            emit found(result_pair);
            results.append(result_pair);
        }
    }
    return results;
}

void Dictionaries::lookupAsync(const QString& word) {
    QtConcurrent::run(
        [this](const QString& word) -> void {
            for (auto& dict_name : dict_names_) {
                auto dict = dicts_.value(dict_name);
                auto result = dict->lookup(word.toStdString());
                if (result != "") {
                    auto dict_basename = QFileInfo(dict_name).baseName();
                    QPair<QString, QString> result_pair(
                        dict_basename,
                        QString::fromStdString(result));
                    emit found(result_pair);
                }
            }
        },
        word);
}

bool Dictionaries::fileCheck(const QString& filename) {
    QFileInfo file_info(filename);
    if (!file_info.exists()) {
        qWarning() << tr("dictionary does not exist: %1").arg(filename);
        return false;
    } else if (!file_info.isFile()) {
        qWarning() << tr("dictionary is not a file: %1").arg(filename);
        return false;
    } else if (!file_info.isReadable()) {
        qWarning() << tr("dictionary is not readable: %1").arg(filename);
        return false;
    }

    return true;
}