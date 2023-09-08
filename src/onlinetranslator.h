#pragma once
#include "abstracttranslator.h"
#include "qonlinetranslator.h"

class OnlineTranslator : public AbstractTranslator {
    Q_OBJECT
   public:
    explicit OnlineTranslator(QObject *parent = nullptr);
    virtual ~OnlineTranslator() = default;
    void translate(const QString &text) override;
    void abort() override;

   private:
    QOnlineTranslator translator_;
    bool retry_flag;
};