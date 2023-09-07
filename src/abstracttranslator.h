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
        QString source_text;
    };

    explicit AbstractTranslator(QObject *parent = nullptr)
        : QObject(parent),
          source_language_(QOnlineTranslator::NoLanguage),
          target_language_(QOnlineTranslator::NoLanguage),
          source_text_() {
        qRegisterMetaType<AbstractTranslator::Result>(
            "AbstractTranslator::Result");
    };

    virtual ~AbstractTranslator() = default;
    virtual void translate(const QString &text) = 0;
    virtual void abort() { emit finished(sourceText()); };

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
    virtual void setSourceText(const QString &text) { source_text_ = text; };
    virtual QString sourceText() const { return source_text_; };

   signals:
    // emit once for each result obtained
    void resultAvailable(AbstractTranslator::Result result);
    // emit after abort() or  all results are obtained
    void finished(QString source_text);

   protected:
    QOnlineTranslator::Language source_language_;
    QOnlineTranslator::Language target_language_;
    QString source_text_;
};
