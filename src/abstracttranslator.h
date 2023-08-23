#pragma once

#include <QObject>
#include <QString>

#include "qonlinetranslator.h"

class AbstractTranslator : public QObject {
    Q_OBJECT

   public:
    struct Result {
        QString title;
        QString content;
    };

    explicit AbstractTranslator(QObject *parent = nullptr)
        : QObject(parent),
          source_language_(QOnlineTranslator::NoLanguage),
          target_language_(QOnlineTranslator::NoLanguage){};

    virtual ~AbstractTranslator() = default;
    virtual void translate(const QString &text) = 0;
    virtual void abort() = 0;

    virtual QOnlineTranslator::Language sourceLanguage() const {
        return source_language_;
    };
    virtual QOnlineTranslator::Language targetLanguage() const {
        return target_language_;
    }
    virtual void setSourceLanguage(QOnlineTranslator::Language lang) {
        source_language_ = lang;
    };
    virtual void setTargetLanguage(QOnlineTranslator::Language lang) {
        target_language_ = lang;
    };
   signals:
    // emit once for each result obtained
    void resultAvailable(AbstractTranslator::Result result);
    // emit when all results are obtained
    void finished();

   protected:
    QOnlineTranslator::Language source_language_;
    QOnlineTranslator::Language target_language_;
};
