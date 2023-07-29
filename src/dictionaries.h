#ifndef DICTIONARIES_H
#define DICTIONARIES_H

#include <QVector>
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
    QVector<QPair<QString,QString>> lookup(const QString& word);
    void lookupAsync(const QString& word);
    void abortLookup();
   signals:
    // void finished(QStringList results);
    // emit every time a search result is found in a dictionary
    void found(QPair<QString,QString> result);

   private:
    bool fileCheck(const QString& filename);
    QMap<QString, QSharedPointer<mdict::Mdict>> dicts_;
    QStringList dict_names_;
};

#endif