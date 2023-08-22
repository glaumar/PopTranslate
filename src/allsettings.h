#pragma once

// #include <QMetaEnum>
#include <QOnlineTranslator>
#include <QStringList>

// get variable name as string, void(Variable) force compiler to check if
// variable exists #define MACRO_VARIABLE_TO_STRING(Variable)
// (void(Variable),#Variable)

class AllSettings {
   public:
    AllSettings()
        : translate_engine(QOnlineTranslator::Engine::Google),
          targetLanguages({QOnlineTranslator::Language::SimplifiedChinese,
                           QOnlineTranslator::Language::English,
                           QOnlineTranslator::Language::Japanese}),
          font(),
          opacity(0.6),
          enable_blur(true),
          enable_auto_copy_translation(true),
          enable_auto_speak(false),
          enable_proxy(false),
          proxy_hostname("localhost"),
          proxy_port(8080),
          enable_auth(false),
          proxy_username(),
          proxy_password(),
          translate_selection_shortcut(Qt::META | Qt::Key_G),
          ocr_shortcut(Qt::CTRL | Qt::META | Qt::Key_G),
          popup_window_size(512, 384),
          show_src_text(false),
          dictionaries(),
          ocr_languages("eng") {}

    QOnlineTranslator::Engine translate_engine;
    QVector<QOnlineTranslator::Language> targetLanguages;
    QFont font;
    qreal opacity;
    bool enable_blur;
    bool enable_auto_copy_translation;
    bool enable_auto_speak;
    bool enable_proxy;
    QString proxy_hostname;
    quint16 proxy_port;
    bool enable_auth;
    QString proxy_username;
    QString proxy_password;
    QKeySequence translate_selection_shortcut;
    QKeySequence ocr_shortcut;
    QSize popup_window_size;
    bool show_src_text;
    QStringList dictionaries;
    QStringList ocr_languages;

    static const AllSettings& defaultSettings() {
        static AllSettings default_settings;
        return default_settings;
    }
};