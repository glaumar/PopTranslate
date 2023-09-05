#pragma once

#include <QOnlineTranslator>
#include <QStringList>
#include "dictionaryinfo.h"


// get variable name as string, void(Variable) force compiler to check if
// variable exists
// #define VAR2STR(Variable) (void(Variable),#Variable)

class AllSettings {
   public:
    AllSettings()
        : translate_engine(QOnlineTranslator::Engine::Google),
          target_languages_({QOnlineTranslator::Language::SimplifiedChinese,
                             QOnlineTranslator::Language::English,
                             QOnlineTranslator::Language::Japanese}),
          active_target_language_(
              QOnlineTranslator::Language::SimplifiedChinese),
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
          dictionaries_info(),
          ocr_languages("eng"),
          monitor_clipboard(false) {}

    QOnlineTranslator::Engine translate_engine;
    QVector<QOnlineTranslator::Language> target_languages_;
    QOnlineTranslator::Language active_target_language_;
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
    QVector<DictionaryInfo> dictionaries_info;
    QStringList ocr_languages;
    bool monitor_clipboard;

    static const AllSettings& defaultSettings() {
        static AllSettings default_settings;
        return default_settings;
    }
};