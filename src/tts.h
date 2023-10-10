#pragma once

#include <QCoroTask>
#include <QMediaPlayer>
#include <QObject>
#include <QOnlineTranslator>
#include <QStateMachine>
#include <QString>

class Tts : public QObject {
    Q_OBJECT
   public:
    explicit Tts(QObject *parent = nullptr);

    QCoro::Task<void> speak(
        const QString text,
        QOnlineTranslator::Language lang = QOnlineTranslator::Auto);
    void stop();

   private:
    void preloadAudio(
        const QString &text,
        QOnlineTranslator::Language lang = QOnlineTranslator::Auto);
    void play();

    QMediaPlayer player_;
    QOnlineTranslator translator_;
};