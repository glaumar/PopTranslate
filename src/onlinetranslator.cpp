#include "onlinetranslator.h"

#include "lang2iso639.h"
#include "poptranslatesettings.h"

OnlineTranslator::OnlineTranslator(QObject *parent)
    : AbstractTranslator(parent), retry_flag(true) {
    connect(&translator_, &QOnlineTranslator::finished, [this] {
        if (translator_.error() == QOnlineTranslator::NoError) {
            // The translation is the same as the source text, switch to
            // another target language, and translate again.
            // Use retry_flag to avoid potentially infinite recursion
            if (retry_flag &&
                translator_.sourceLanguage() == targetLanguage() &&
                translator_.translation().trimmed() ==
                    translator_.source().trimmed()) {
                retry_flag = false;
                for (auto lang :
                     PopTranslateSettings::instance().targetLanguages()) {
                    if (lang != targetLanguage()) {
                        setTargetLanguage(lang);
                        break;
                    }
                }
                translate(translator_.source());
                return;
            } else {
                qDebug() << tr("OnlineTranslator Success");

                QString title;
                if (!retry_flag) {
                    title = QString("%1 (%2)").arg(
                        PopTranslateSettings::instance().translateEngineStr(),
                        Lang2ISO639(targetLanguage()));
                } else {
                    title =
                        PopTranslateSettings::instance().translateEngineStr();
                }

                OnlineTranslator::Result result{title,
                                                translator_.translation(),
                                                translator_.source()};
                emit resultAvailable(result);
                retry_flag = true;
                emit finished(translator_.source());
            }

        } else {
            auto error_msg = tr("OnlineTranslator Failed: %1")
                                 .arg(translator_.errorString());
            qWarning() << error_msg;
            retry_flag = true;
            emit finished(translator_.source());
        }
    });
}

void OnlineTranslator::translate(const QString &text) {
    if (text.isEmpty()) {
        emit finished(text);
        return;
    }

    // setSourceText(text);

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