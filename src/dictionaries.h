#pragma once

#include <QFuture>
#include <QMap>
#include <QSharedPointer>
#include <QStringList>
#include <QVector>
#include <atomic>
#include <QReadWriteLock>

#include "abstracttranslator.h"
#include "mdict.h"

class Dictionaries : public AbstractTranslator {
    Q_OBJECT
   public:
    explicit Dictionaries(QObject* parent = nullptr);
    virtual ~Dictionaries() = default;
    void translate(const QString& text) override;
    void abort() override;

    void clear();
    void addDict(const QString& filename);
    void addDicts(const QStringList& filenames);
    void setDicts(const QStringList& filenames);
    void setDictsAsync(const QStringList& filenames);
    void removeDict(const QString& filename);
    void removeDicts(const QStringList& filenames);
    void lookup(const QString& text);

   private:
    bool fileCheck(const QString& filename);
    QMap<QString, QSharedPointer<mdict::Mdict>> dicts_;
    QStringList dict_names_;
    std::atomic_bool abort_;
    QReadWriteLock lock_;
};