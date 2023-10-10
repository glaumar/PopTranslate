#include "tts.h"

#include <QCoroCore>
#include <QMediaPlaylist>

#include "poptranslatesettings.h"
#include "qonlinetranslator.h"
#include "qonlinetts.h"

Tts::Tts(QObject *parent) : QObject(parent) {
    QMediaPlaylist *playlist = new QMediaPlaylist(&player_);
    player_.setPlaylist(playlist);
}

void Tts::preloadAudio(const QString &text, QOnlineTranslator::Language lang) {
    if (text.isEmpty()) {
        return;
    }

    auto engine = PopTranslateSettings::instance().translateEngine();
    if (engine != QOnlineTranslator::Google &&
        engine != QOnlineTranslator::Yandex) {
        engine = QOnlineTranslator::Google;
    }

    QOnlineTts onlinetts;
    onlinetts.generateUrls(text, engine, lang);

    if (onlinetts.error() != QOnlineTts::NoError) {
        qWarning() << tr("TTS: %1").arg(onlinetts.errorString());
        return;
    }

    player_.playlist()->clear();
    player_.playlist()->addMedia(onlinetts.media());

    // TODO: cache audio
}

void Tts::play() { player_.play(); }

QCoro::Task<void> Tts::speak(const QString text,
                             QOnlineTranslator::Language lang) {
    stop();

    if (text.isEmpty()) {
        co_return;
    }

    // detect language
    if (lang == QOnlineTranslator::Auto) {
        translator_.detectLanguage(text);
        co_await qCoro(&translator_, &QOnlineTranslator::finished);

        if (translator_.error() != QOnlineTranslator::NoError) {
            qWarning() << tr("TTS: %1").arg(translator_.errorString());
            co_return;
        }

        lang = translator_.sourceLanguage();
    }

    preloadAudio(text, lang);
    play();
}

void Tts::stop() { player_.stop(); }