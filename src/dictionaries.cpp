#include "dictionaries.h"

#include <QDebug>
#include <QFileInfo>

#include "poptranslatesettings.h"

// macro “F” definition conflicts
#undef F
#include <QThread>
#include <QtConcurrent>

Dictionaries::Dictionaries(QObject* parent)
    : AbstractTranslator(parent),
      abort_(false),
      lock_(QReadWriteLock::Recursive) {
    setDictsAsync(PopTranslateSettings::instance().dictionaries());
    connect(&PopTranslateSettings::instance(),
            &PopTranslateSettings::dictionariesChanged,
            this,
            &Dictionaries::setDictsAsync);
}

void Dictionaries::clear() {
    QWriteLocker locker(&lock_);
    dicts_.clear();
    dict_info_vec_.clear();
}

void Dictionaries::addDict(const DictionaryInfo& dict_info) {
    QWriteLocker locker(&lock_);
    if (dicts_.contains(dict_info)) {
        qDebug() << tr("dictionary already exists: %1")
                        .arg(dict_info.filename);  // TODO: maybe remove
        return;
    }

    if (!fileCheck(dict_info.filename)) {
        return;
    }

    // qDebug() << tr("addDict: %1").arg(dict_info.filename);
    QSharedPointer<mdict::Mdict> md_p(
        new mdict::Mdict(dict_info.filename.toStdString()));
    md_p->init();
    qDebug() << tr("%1 load success").arg(dict_info.filename);
    dicts_.insert(dict_info, md_p);
    dict_info_vec_.append(dict_info);
}

void Dictionaries::addDicts(const QVector<DictionaryInfo>& info_vec) {
    for (auto& info : info_vec) {
        addDict(info);
    }
}

void Dictionaries::setDicts(const QVector<DictionaryInfo>& info_vec) {
    QWriteLocker locker(&lock_);
    QSet<DictionaryInfo> old_set(dict_info_vec_.begin(), dict_info_vec_.end());
    QSet<DictionaryInfo> new_set(info_vec.begin(), info_vec.end());
    auto diff = old_set - new_set;

    removeDicts(QVector<DictionaryInfo>(diff.begin(), diff.end()));
    addDicts(info_vec);

    dict_info_vec_ = info_vec;
    QMutableVectorIterator<DictionaryInfo> it(dict_info_vec_);
    while (it.hasNext()) {
        if (!dicts_.contains(it.next())) {
            it.remove();
        }
    };
}

void Dictionaries::setDictsAsync(const QVector<DictionaryInfo>& info_vec) {
    QtConcurrent::run(this, &Dictionaries::setDicts, info_vec);
}

void Dictionaries::removeDict(const DictionaryInfo& dict_info) {
    QWriteLocker locker(&lock_);
    if (!dicts_.contains(dict_info)) {
        return;
    }

    dicts_.remove(dict_info);
    for (int i = 0; i < dict_info_vec_.size(); i++) {
        if (dict_info_vec_[i] == dict_info) {
            dict_info_vec_.removeAt(i);
            qDebug() << tr("removeDict: %1").arg(dict_info.filename);
            break;
        }
    }
}

void Dictionaries::removeDicts(const QVector<DictionaryInfo>& info_vec) {
    for (auto& info : info_vec) {
        removeDict(info);
    }
}

void Dictionaries::translate(const QString& text) {
    QtConcurrent::run(this, &Dictionaries::lookup, text);
}

void Dictionaries::abort() { abort_ = true; }

void Dictionaries::lookup(const QString& text) {
    abort_ = false;

    QReadLocker locker(&lock_);
    for (auto it = dict_info_vec_.begin();
         it != dict_info_vec_.end() && !abort_;
         ++it) {
        if (it->target_language != QOnlineTranslator::Auto &&
            it->target_language != targetLanguage()) {
            continue;
        }

        auto dict = dicts_.value(*it);
        auto content = dict->lookup(text.toStdString());
        if (content != "") {
            QString content_qstr = QString::fromStdString(content);
            content_qstr.remove(QRegExp("`[0-9]`|</?br>"));
            auto dict_basename = QFileInfo(it->filename).baseName();
            AbstractTranslator::Result result{dict_basename,
                                              content_qstr,
                                              text};
            if (abort_) {
                break;
            }
            emit resultAvailable(result);
            qDebug()
                << tr("Dictionaries Lookup Success: %1").arg(dict_basename);
        }
    }
    emit finished(text);
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