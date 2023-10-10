#include "onlinetranslator.h"

#include <QCoroCore>

#include "lang2iso639.h"
#include "poptranslatesettings.h"

OnlineTranslator::OnlineTranslator(QObject *parent)
    : AbstractTranslator(parent) {}

QCoro::AsyncGenerator<AbstractTranslator::Result> OnlineTranslator::translate(
    const QString &text) {
    if (text.isEmpty()) {
        co_return;
    }

    auto result = co_await translateCoro(text);

    // The translation is the same as the source text, switch to
    // another target language, and translate again.
    if (translator_.sourceLanguage() == targetLanguage() &&
        translator_.translation().trimmed() == translator_.source().trimmed()) {
        // set another target language
        for (auto lang : PopTranslateSettings::instance().targetLanguages()) {
            if (lang != targetLanguage()) {
                setTargetLanguage(lang);
                break;
            }
        }

        // translate again
        result = co_await translateCoro(text);
    }

    co_yield result;
}

QCoro::Task<AbstractTranslator::Result> OnlineTranslator::translateCoro(
    const QString &text) {
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

    co_await qCoro(&translator_, &QOnlineTranslator::finished);

    if (translator_.error() != QOnlineTranslator::NoError) {
        auto error_msg =
            tr("OnlineTranslator Failed: %1").arg(translator_.errorString());
        qWarning() << error_msg;
    } else {
        QString title = PopTranslateSettings::instance().translateEngineStr();
        QString html_result =
            QString(R"(<!DOCTYPE html><html><body><div>%1</div></body></html>)")
                .arg(translator_.translation());
        co_return OnlineTranslator::Result{title, html_result};
    }
}
