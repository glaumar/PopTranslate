#pragma once

#include <QFuture>
#include <QMap>
#include <QSharedPointer>
#include <QStringList>
#include <QVector>

#include "abstracttranslator.h"
#include "dictionaryinfo.h"
#include "pybind11_wrap.h"

// using __attribute__((visibility("default"))) to remove the warning (https://github.com/vgc/vgc/issues/11)
class __attribute__((visibility("default")))  MDict : public AbstractTranslator {
    Q_OBJECT
   public:
    explicit MDict(QObject* parent = nullptr);
    virtual ~MDict() = default;
    void translate(const QString& text) override;
    void abort() override;

    void clear();
    void addDict(const DictionaryInfo& dict_info);
    void addDicts(const QVector<DictionaryInfo>& info_vec);
    void setDicts(const QVector<DictionaryInfo>& info_vec);
    void removeDict(const DictionaryInfo& dict_info);
    void removeDicts(const QVector<DictionaryInfo>& info_vec);
    void lookup(const QString& text);

   private:
    pybind11::scoped_interpreter py_guard_;
    pybind11::object index_builder_;
    bool fileCheck(const QString& filename);
    QMap<DictionaryInfo, pybind11::object> dicts_;
    QVector<DictionaryInfo> dict_info_vec_;
};