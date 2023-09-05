#pragma once

#include <QFuture>
#include <QMap>
#include <QReadWriteLock>
#include <QSharedPointer>
#include <QStringList>
#include <QVector>
#include <atomic>

#include "abstracttranslator.h"
#include "mdict.h"
#include "dictionaryinfo.h"

class Dictionaries : public AbstractTranslator {
    Q_OBJECT
   public:
    explicit Dictionaries(QObject* parent = nullptr);
    virtual ~Dictionaries() = default;
    void translate(const QString& text) override;
    void abort() override;

    void clear();
    void addDict(const DictionaryInfo& dict_info);
    void addDicts(const QVector<DictionaryInfo>& info_vec);
    void setDicts(const QVector<DictionaryInfo>& info_vec);
    void setDictsAsync(const QVector<DictionaryInfo>& info_vec);
    void removeDict(const DictionaryInfo& dict_info);
    void removeDicts(const QVector<DictionaryInfo>& info_vec);
    void lookup(const QString& text);

   private:
    bool fileCheck(const QString& filename);
    QMap<DictionaryInfo, QSharedPointer<mdict::Mdict>> dicts_;
    QVector<DictionaryInfo> dict_names_;
    std::atomic_bool abort_;
    QReadWriteLock lock_;
};