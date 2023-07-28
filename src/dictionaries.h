#ifndef DICTIONARIES_H
#define DICTIONARIES_H

#include <QList>
#include <QMap>
#include <QSharedPointer>
#include <QStringList>

#include "mdict.h"

class Dictionaries : public QObject {
    Q_OBJECT
   public:
    Dictionaries();
    ~Dictionaries();
    void clear();
    void addDict(const QString& filename);
    void addDicts(const QStringList& filenames);
    void setDicts(const QStringList& filenames);
    void removeDict(const QString& filename);
    void removeDicts(const QStringList& filenames);
    QStringList lookup(const QString& word);
    void lookupAsync(const QString& word);
    void abortLookup();
   signals:
    // void finished(QStringList results);
    // emit every time a search result is found in a dictionary
    void found(QString result);

   private:
    QMap<QString, QSharedPointer<mdict::Mdict>> dicts_;
    QStringList dict_names_;
};

#endif