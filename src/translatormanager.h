#pragma once

#include <QCoroTask>
#include <QObject>
#include <QPair>
#include <QVector>

#include "abstracttranslator.h"

class TranslatorManager : public QObject {
    Q_OBJECT
   public:
    explicit TranslatorManager(QObject* parent = nullptr);
    void addTranslator(AbstractTranslator* translator);
   public slots:
    void setSourceLanguage(QOnlineTranslator::Language language);
    void setTargetLanguage(QOnlineTranslator::Language language);
    void translate(const QString& text);
   signals:
    void resultAvailable(AbstractTranslator::Result result);

   private:
    QCoro::Task<void> translateCoro(AbstractTranslator* t, const QString text);

    QVector<AbstractTranslator*> translators_;
    QOnlineTranslator::Language target_language_;
    QOnlineTranslator::Language source_language_;
    QString source_text_;
};