#pragma once

#include <QMap>
#include <QSharedPointer>
#include <QStringList>
#include <QVector>

#include "abstracttranslator.h"
#include "mdict.h"

class Dictionaries : public AbstractTranslator {
    Q_OBJECT
   public:
    explicit Dictionaries(QObject* parent = nullptr);
    virtual ~Dictionaries() = default;
    void translate(const QString& text) override;
    void abort() override;

   public slots:
    void clear();
    void addDict(const QString& filename);
    void addDicts(const QStringList& filenames);
    void setDicts(const QStringList& filenames);
    void removeDict(const QString& filename);
    void removeDicts(const QStringList& filenames);
    void lookup(const QString& word);

   private:
    bool fileCheck(const QString& filename);
    QMap<QString, QSharedPointer<mdict::Mdict>> dicts_;
    QStringList dict_names_;
    bool is_aborted_;
};