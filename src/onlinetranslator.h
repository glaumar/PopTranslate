#pragma once
#include <QCoroTask>

#include "abstracttranslator.h"
#include "qonlinetranslator.h"

class OnlineTranslator : public AbstractTranslator {
    Q_OBJECT
   public:
    explicit OnlineTranslator(QObject *parent = nullptr);
    virtual ~OnlineTranslator() = default;
    QCoro::AsyncGenerator<AbstractTranslator::Result> translate(
        const QString &text) override;

   private:
    QCoro::Task<AbstractTranslator::Result> translateCoro(const QString &text);

    QOnlineTranslator translator_;
};