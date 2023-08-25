#include "tts.h"

#include <QMediaPlaylist>

#include "poptranslatesettings.h"
#include "qonlinetranslator.h"
#include "qonlinetts.h"

Tts::Tts(QObject *parent) : QObject(parent) {
    QMediaPlaylist *playlist = new QMediaPlaylist(&player_);
    player_.setPlaylist(playlist);

    // detect language finished
    connect(&translator_, &QOnlineTranslator::finished, this, [this] {
        if (translator_.error() == QOnlineTranslator::NoError) {
            speak(text_, translator_.sourceLanguage());
        } else {
            qWarning() << tr("TTS: %1").arg(translator_.errorString());
        }
    });
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

void Tts::speak(const QString &text, QOnlineTranslator::Language lang) {
    stop();

    if (text.isEmpty()) {
        return;
    }

    if (lang == QOnlineTranslator::Auto) {
        text_ = text;
        // recall this function when detect language finished
        translator_.detectLanguage(text);
    } else {
        preloadAudio(text, lang);
        play();
    }
}

void Tts::stop() { player_.stop(); }