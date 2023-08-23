#include "onlinetranslator.h"

#include "poptranslatesettings.h"

OnlineTranslator::OnlineTranslator(QObject *parent)
    : AbstractTranslator(parent) {
    connect(&translator_, &QOnlineTranslator::finished, [this] {
        if (translator_.error() == QOnlineTranslator::NoError) {
            qDebug() << tr("OnlineTranslator Success");
            OnlineTranslator::Result result{
                PopTranslateSettings::instance().translateEngineStr(),
                translator_.translation()};
            emit resultAvailable(result);
        } else {
            auto error_msg = tr("OnlineTranslator Failed: %1")
                                 .arg(translator_.errorString());
            qWarning() << error_msg;
        }
        emit finished();
    });
}

void OnlineTranslator::translate(const QString &text) {
    if (text.isEmpty()) {
        return;
    }

    if (sourceLanguage() == QOnlineTranslator::NoLanguage) {
        setSourceLanguage(QOnlineTranslator::Auto);
    }

    if (targetLanguage() == QOnlineTranslator::NoLanguage) {
        setTargetLanguage(
            PopTranslateSettings::instance().activeTargetLanguage());
    }

    translator_.translate(text,
                          PopTranslateSettings::instance().translateEngine(),
                          targetLanguage(),
                          sourceLanguage());
}

void OnlineTranslator::abort() { translator_.abort(); }
