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
    dict_names_.clear();
}

void Dictionaries::addDict(const QString& filename) {
    QWriteLocker locker(&lock_);
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
    qDebug() << tr("%1 load success").arg(filename);
    dicts_.insert(filename, md_p);
    dict_names_.append(filename);
}

void Dictionaries::addDicts(const QStringList& filenames) {
    for (auto& filename : filenames) {
        addDict(filename);
    }
}

void Dictionaries::setDicts(const QStringList& filenames) {
    QWriteLocker locker(&lock_);
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

void Dictionaries::setDictsAsync(const QStringList& filenames) {
    QtConcurrent::run(this, &Dictionaries::setDicts, filenames);
}

void Dictionaries::removeDict(const QString& filename) {
    QWriteLocker locker(&lock_);
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

void Dictionaries::translate(const QString& text) {
    QtConcurrent::run(this, &Dictionaries::lookup, text);
}

void Dictionaries::abort() { abort_ = true; }

void Dictionaries::lookup(const QString& text) {
    abort_ = false;

    QReadLocker locker(&lock_);
    for (auto it = dict_names_.begin(); it != dict_names_.end() && !abort_;
         ++it) {
        auto dict = dicts_.value(*it);
        auto content = dict->lookup(text.toStdString());
        if (content != "") {
            QString content_qstr = QString::fromStdString(content);
            content_qstr.remove(QRegExp("`[0-9]`|</?br>"));
            auto dict_basename = QFileInfo(*it).baseName();
            AbstractTranslator::Result result{dict_basename, content_qstr};
            if (abort_) {
                break;
            }
            emit resultAvailable(result);
            qDebug()
                << tr("Dictionaries Lookup Success: %1").arg(dict_basename);
        }
    }
    emit finished();
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