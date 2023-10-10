#include "mdict.h"

#include <QCoroFuture>
#include <QDebug>
#include <QFileInfo>
#include <QScopedPointer>
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

    setDicts(PopTranslateSettings::instance().dictionaries());
    connect(&PopTranslateSettings::instance(),
            &PopTranslateSettings::dictionariesChanged,
            this,
            &MDict::setDicts);
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

QCoro::Task<void> MDict::addDict(const DictionaryInfo& dict_info) {
    if (dicts_.contains(dict_info)) {
        co_return;
    }

    if (!fileCheck(dict_info.filename)) {
        co_return;
    }

    // Since dictionary loading is asynchronous, dictionary information is added
    // to dict_info_vec_ first, and then the dictionary is loaded. This ensures
    // that the query order of the dictionary is consistent with the order of
    // addition.
    dict_info_vec_.append(dict_info);

    // load dictionary
    py::object* md = co_await QtConcurrent::run([this, dict_info]() {
        py::gil_scoped_acquire acquire;
        py::object* md = new py::object();
        *md = index_builder_(dict_info.filename.toStdString());
        py::gil_scoped_release release;
        return md;
    });
    py::gil_scoped_acquire acquire;
    QScopedPointer<py::object> md_ptr(std::move(md));

    dicts_.insert(dict_info, *md_ptr);
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

QCoro::AsyncGenerator<AbstractTranslator::Result> MDict::translate(
    const QString& text) {
    auto lookup_coro = [this](const DictionaryInfo& info,
                              const QString& text) -> QCoro::Task<QString> {
        auto result =
            co_await QtConcurrent::run(this, &MDict::lookup, info, text);
        co_return result;
    };

    for (auto it = dict_info_vec_.begin(); it != dict_info_vec_.end(); ++it) {
        if (it->target_language != QOnlineTranslator::Auto &&
            it->target_language != targetLanguage()) {
            continue;
        }

        if (!dicts_.contains(*it)) {
            continue;
        }

        QString result = co_await lookup_coro(*it, text);

        if (!result.isEmpty()) {
            // auto dereference
            if (result.startsWith("@@@LINK=")) {
                auto link_word = result.replace("@@@LINK=", "").trimmed();
                result = co_await lookup_coro(*it, link_word);
            }

            auto dict_basename = QFileInfo(it->filename).baseName();
            co_yield AbstractTranslator::Result{dict_basename, toHtml(result)};
        }
    }
}

QString MDict::lookup(const DictionaryInfo& info, const QString& text) {
    py::gil_scoped_acquire acquire;
    py::object md = dicts_.value(info);

    // If “ignorecase = True” is used, the query speed will be very slow
    py::list results =
        md.attr("mdx_lookup")(text.toStdString() /*, "ignorecase = True"*/);

    if (results.empty()) {
        return QString();
    }
    // TODO: support multiple results
    return QString::fromStdString(results[0].cast<std::string>());
}

QString MDict::toHtml(const QString& text) {
    static const QString html_template =
        R"(<!DOCTYPE html><html><body><div>%1</div></body></html>)";
    static const QStringList tag_template = {
        R"(%1)",
        R"(<font size="+1"><b>%1</b></font>)",
        R"(<font><br>%1</font>)",
        R"(<font color="dodgerblue">%1</font>)",
        R"(<font color="gray" size="-1">%1</font>)",
        R"(<font color="orange">%1</font>)",
        R"(<font>%1</font>)",
        R"(%1)"};

    QStringList result;

    QRegExp rx("`([0-9])`([^`]*)");

    int pos = rx.indexIn(text, pos);
    if (pos == -1) {
        auto result = text;
        return html_template.arg(result.replace("</br>", "<br>"));
    }

    do {
        int index = rx.cap(1).toInt();
        if (index < 0 || index >= tag_template.size()) {
            index = 0;
        }
        result << tag_template[index].arg(rx.cap(2).replace("</br>", "<br>"));
        pos += rx.matchedLength();
        pos = rx.indexIn(text, pos);
    } while (pos != -1);

    return result.join("");
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