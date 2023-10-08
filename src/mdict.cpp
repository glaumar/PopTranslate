#include "mdict.h"

#include <QDebug>
#include <QFileInfo>
#include <QStandardPaths>
#include <QtConcurrent>

#include "poptranslatesettings.h"

namespace py = pybind11;

MDict::MDict(QObject* parent) : AbstractTranslator(parent) {
    py::gil_scoped_acquire acquire;
    py::module_ sys = py::module_::import("sys");
    QString mdict_query_path =
        QStandardPaths::locate(QStandardPaths::AppDataLocation,
                               "mdict-query",
                               QStandardPaths::LocateDirectory);

    sys.attr("path").attr("append")(mdict_query_path.toStdString());
    index_builder_ = py::module_::import("mdict_query").attr("IndexBuilder");

    setDictsAsync(PopTranslateSettings::instance().dictionaries());
    connect(&PopTranslateSettings::instance(),
            &PopTranslateSettings::dictionariesChanged,
            this,
            &MDict::setDictsAsync);
}

MDict::~MDict() {
    clear();
    py::gil_scoped_acquire acquire;
    index_builder_ = py::none();
    index_builder_.release();
}

void MDict::clear() {
    py::gil_scoped_acquire acquire;
    dicts_.clear();
    dict_info_vec_.clear();
}

void MDict::addDict(const DictionaryInfo& dict_info) {
    py::gil_scoped_acquire acquire;
    if (dicts_.contains(dict_info)) {
        return;
    }

    if (!fileCheck(dict_info.filename)) {
        return;
    }

    py::object md = index_builder_(dict_info.filename.toStdString());
    dicts_.insert(dict_info, md);
    dict_info_vec_.append(dict_info);

    qDebug() << tr("%1 load success").arg(dict_info.filename);
}

void MDict::addDicts(const QVector<DictionaryInfo>& info_vec) {
    for (auto& info : info_vec) {
        addDict(info);
    }
}

void MDict::setDicts(const QVector<DictionaryInfo>& info_vec) {
    clear();
    addDicts(info_vec);
}

void MDict::setDictsAsync(const QVector<DictionaryInfo>& info_vec) {
    QtConcurrent::run(this, &MDict::setDicts, info_vec);
}

void MDict::removeDict(const DictionaryInfo& dict_info) {
    if (!dicts_.contains(dict_info)) {
        return;
    }

    py::gil_scoped_acquire acquire;
    dicts_.remove(dict_info);
    for (int i = 0; i < dict_info_vec_.size(); i++) {
        if (dict_info_vec_[i] == dict_info) {
            dict_info_vec_.removeAt(i);
            qDebug() << tr("removeDict: %1").arg(dict_info.filename);
            break;
        }
    }
}

void MDict::removeDicts(const QVector<DictionaryInfo>& info_vec) {
    for (auto& info : info_vec) {
        removeDict(info);
    }
}

void MDict::translate(const QString& text) {
    QtConcurrent::run(this, &MDict::lookup, text);
}

void MDict::abort() { /*abort_ = true; */
}

void MDict::lookup(const QString& text) {
    py::gil_scoped_acquire acquire;
    for (auto it = dict_info_vec_.begin(); it != dict_info_vec_.end(); ++it) {
        if (it->target_language != QOnlineTranslator::Auto &&
            it->target_language != targetLanguage()) {
            continue;
        }

        py::object md = dicts_.value(*it);
        py::list results = md.attr("mdx_lookup")(text.toStdString());
        if (results.empty()) {
            continue;
        }
        auto content = results[0].cast<std::string>();
        if (content != "") {
            QString content_qstr = QString::fromStdString(content);
            content_qstr.remove(QRegExp("`[0-9]`|</?br>"));
            auto dict_basename = QFileInfo(it->filename).baseName();
            AbstractTranslator::Result result{dict_basename,
                                              content_qstr,
                                              text};
            emit resultAvailable(result);
            qDebug()
                << tr("Dictionaries Lookup Success: %1").arg(dict_basename);
        }
    }
    emit finished(text);
}

bool MDict::fileCheck(const QString& filename) {
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