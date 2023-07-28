#include "dictionaries.h"

#include <QDebug>

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
        return;
    }
    qDebug() << "addDict:" << filename;
    QSharedPointer<mdict::Mdict> md_p(new mdict::Mdict(filename.toStdString()));
    md_p->init();
    dicts_.insert(filename, md_p);
    dict_names_.append(filename);
}
void Dictionaries::addDicts(const QList<QString>& filenames) {
    for (auto& filename : filenames) {
        addDict(filename);
    }
    dict_names_ = filenames;
}

void Dictionaries::removeDict(const QString& filename) {
    if (!dicts_.contains(filename)) {
        return;
    }

    dicts_.remove(filename);
    for (int i = 0; i < dict_names_.size(); i++) {
        if (dict_names_[i] == filename) {
            dict_names_.removeAt(i);
            break;
        }
    }
}
void Dictionaries::removeDicts(const QList<QString>& filenames) {
    for (auto& filename : filenames) {
        removeDict(filename);
    }
}

QList<QString> Dictionaries::lookup(const QString& word) {
    qDebug() << "lookup:" << word;
    QList<QString> results;
    for (auto& dict_name : dict_names_) {
        qDebug() << "dict_name:" << dict_name;
        auto dict = dicts_.value(dict_name);
        auto result = dict->lookup(word.toStdString());
        if (result != "") {
            qDebug() << "found:" << QString::fromStdString(result);
            emit found(QString::fromStdString(result));
            results.append(QString::fromStdString(result));
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
                    emit found(QString::fromStdString(result));
                }
            }
        },
        word);
}