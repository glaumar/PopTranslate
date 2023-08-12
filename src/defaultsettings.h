#pragma once

#include <QMetaEnum>
#include <QOnlineTranslator>
#include <QStringList>

// get variable name as string, void(Variable) force compiler to check if
// variable exists #define MACRO_VARIABLE_TO_STRING(Variable)
// (void(Variable),#Variable)

class DefaultSettings {
   public:
    DefaultSettings()
        : translate_engine(QOnlineTranslator::Engine::Google),
          target_language_1(QOnlineTranslator::Language::SimplifiedChinese),
          target_language_2(QOnlineTranslator::Language::English),
          target_language_3(QOnlineTranslator::Language::Japanese),
          font(),
          opacity(0.6),
          enable_blur(true),
          enable_proxy(false),
          proxy_hostname("localhost"),
          proxy_port(8080),
          enable_auth(false),
          proxy_username(),
          proxy_password(),
          shortcut_popup_main(Qt::META | Qt::Key_G),
        //   shortcut_popup_alt(Qt::CTRL | Qt::META | Qt::Key_G),
          popup_window_size(512, 384),
          show_src_text(false),
          dictionaries(),
          ocr_languages("eng") {}

    QOnlineTranslator::Engine translate_engine;
    QOnlineTranslator::Language target_language_1;
    QOnlineTranslator::Language target_language_2;
    QOnlineTranslator::Language target_language_3;
    QFont font;
    qreal opacity;
    bool enable_blur;
    bool enable_proxy;
    QString proxy_hostname;
    quint16 proxy_port;
    bool enable_auth;
    QString proxy_username;
    QString proxy_password;
    QKeySequence shortcut_popup_main;
    // QKeySequence shortcut_popup_alt;
    QSize popup_window_size;
    bool show_src_text;
    QStringList dictionaries;
    QStringList ocr_languages;

    inline const char* translate_engine_to_str() const {
        return enumValueToKey(translate_engine);
    }

    inline const char* target_language_1_to_str() const {
        return enumValueToKey(target_language_1);
    }

    inline const char* target_language_2_to_str() const {
        return enumValueToKey(target_language_2);
    }

    inline const char* target_language_3_to_str() const {
        return enumValueToKey(target_language_3);
    }

    template <class T>
    static inline const char* enumValueToKey(const T value) {
        return QMetaEnum::fromType<T>().valueToKey(value);
    }
};