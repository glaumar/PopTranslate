#include "poptranslatesettings.h"

#include <QDebug>

#include "poptranslate.h"

PopTranslateSettings::PopTranslateSettings()
    : settings_(APPLICATION_NAME, APPLICATION_NAME) {
    qRegisterMetaTypeStreamOperators<DictionaryInfo>("DictionaryInfo");
    qRegisterMetaTypeStreamOperators<QVector<DictionaryInfo>>(
        "QVector<DictionaryInfo>");
    load();
}

void PopTranslateSettings::load() {
    if (settings_.status() != QSettings::NoError) {
        qWarning() << "Settings: Failed to load settings";
    }

    const AllSettings& default_settings = AllSettings::defaultSettings();
    // If the key is not defined, use the default value

    all_.translate_engine =
        settings_.value("translate_engine", default_settings.translate_engine)
            .value<QOnlineTranslator::Engine>();

    auto languages = settings_.value("target_languages").value<QStringList>();

    if (languages.isEmpty()) {
        all_.target_languages_ = default_settings.target_languages_;
    } else {
        all_.target_languages_ = stringListToTargetLanguages(languages);
    }

    // active_target_language_ not saved in config file
    all_.active_target_language_ = all_.target_languages_[0];

    all_.font = settings_.value("font", default_settings.font).value<QFont>();
    all_.opacity =
        settings_.value("opacity", default_settings.opacity).value<qreal>();
    all_.enable_blur =
        settings_.value("enable_blur", default_settings.enable_blur)
            .value<bool>();
    all_.enable_auto_copy_translation =
        settings_
            .value("enable_auto_copy_translation",
                   default_settings.enable_auto_copy_translation)
            .value<bool>();
    all_.enable_auto_speak =
        settings_.value("enable_auto_speak", default_settings.enable_auto_speak)
            .value<bool>();
    all_.enable_proxy =
        settings_.value("enable_proxy", default_settings.enable_proxy)
            .value<bool>();
    all_.proxy_hostname =
        settings_.value("proxy_hostname", default_settings.proxy_hostname)
            .value<QString>();
    all_.proxy_port = settings_.value("proxy_port", default_settings.proxy_port)
                          .value<quint16>();
    all_.enable_auth =
        settings_.value("enable_auth", default_settings.enable_auth)
            .value<bool>();
    all_.proxy_username =
        settings_.value("proxy_username", default_settings.proxy_username)
            .value<QString>();
    all_.proxy_password =
        settings_.value("proxy_password", default_settings.proxy_password)
            .value<QString>();
    all_.translate_selection_shortcut =
        settings_
            .value("translate_selection_shortcut",
                   default_settings.translate_selection_shortcut)
            .value<QKeySequence>();
    all_.ocr_shortcut =
        settings_.value("ocr_shortcut", default_settings.ocr_shortcut)
            .value<QKeySequence>();
    all_.popup_window_size =
        settings_.value("popup_window_size", default_settings.popup_window_size)
            .value<QSize>();
    // all_.show_src_text =
    //     settings_.value("show_src_text", default_settings.show_src_text)
    //         .value<bool>();
    all_.dictionaries_info =
        settings_
            .value("dictionaries_info",
                   QVariant::fromValue(default_settings.dictionaries_info))
            .value<QVector<DictionaryInfo>>();
    all_.ocr_languages =
        settings_.value("ocr_languages", default_settings.ocr_languages)
            .value<QStringList>();
}

void PopTranslateSettings::setTranslateEngine(
    QOnlineTranslator::Engine engine) {
    all_.translate_engine = engine;
    settings_.setValue("translate_engine", translateEngineStr());
    emit translateEngineChanged(engine);
    qDebug() << tr("Settings: Change translate_engine to %1")
                    .arg(translateEngineStr());
};

void PopTranslateSettings::setTargetLanguages(
    QVector<QOnlineTranslator::Language> languages) {
    all_.target_languages_ = languages;
    settings_.setValue("target_languages", targetLanguagesStr());
    emit targetLanguagesChanged(languages);
    qDebug() << tr("Settings: set target languages : %1")
                    .arg(targetLanguagesStr().join(" "));
}

void PopTranslateSettings::setActiveTargetLanguage(
    QOnlineTranslator::Language language) {
    all_.active_target_language_ = language;
    emit activeTargetLanguageChanged(language);
    qDebug() << tr("Settings: set active target language : %1")
                    .arg(targetLanguageStr(language));
}

void PopTranslateSettings::setFont(QFont font) {
    all_.font = font;
    settings_.setValue("font", font);
    emit fontChanged(font);
    qDebug() << tr("Settings: Change font to %1 %2 %3")
                    .arg(font.family(), font.styleName())
                    .arg(font.pointSize());
};

void PopTranslateSettings::setOpacity(qreal opacity) {
    all_.opacity = opacity;
    settings_.setValue("opacity", opacity);
    emit opacityChanged(opacity);
    qDebug() << tr("Settings: Change opacity to %1").arg(opacity);
};

void PopTranslateSettings::setEnableBlur(bool enable) {
    all_.enable_blur = enable;
    settings_.setValue("enable_blur", enable);
    emit enableBlurChanged(enable);
    qDebug() << tr("Settings: %1 blur effect")
                    .arg(enable ? tr("Enable") : tr("Disable"));
};

void PopTranslateSettings::setEnableAutoCopyTranslation(bool enable) {
    all_.enable_auto_copy_translation = enable;
    settings_.setValue("enable_auto_copy_translation", enable);
    emit enableAutoCopyTranslationChanged(enable);
    qDebug() << tr("Settings: %1 auto copy translation")
                    .arg(enable ? tr("Enable") : tr("Disable"));
};

void PopTranslateSettings::setEnableAutoSpeak(bool enable) {
    all_.enable_auto_speak = enable;
    settings_.setValue("enable_auto_speak", enable);
    emit enableAutoSpeakChanged(enable);
    qDebug() << tr("Settings: %1 auto speak")
                    .arg(enable ? tr("Enable") : tr("Disable"));
}

void PopTranslateSettings::setEnableProxy(bool enable) {
    all_.enable_proxy = enable;
    settings_.setValue("enable_proxy", enable);
    emit enableProxyChanged(enable);
    qDebug() << tr("Settings: %1 proxy %2:%3")
                    .arg(enable ? tr("Enable") : tr("Disable"))
                    .arg(all_.proxy_hostname)
                    .arg(all_.proxy_port);
};

void PopTranslateSettings::setProxyHostname(QString hostname) {
    all_.proxy_hostname = hostname;
    settings_.setValue("proxy_hostname", hostname);
    emit proxyHostnameChanged(hostname);
    qDebug() << tr("Settings: Change proxy hostname to %1").arg(hostname);
};

void PopTranslateSettings::setProxyPort(quint16 port) {
    all_.proxy_port = port;
    settings_.setValue("proxy_port", port);
    emit proxyPortChanged(port);
    qDebug() << tr("Settings: Change proxy port to %1").arg(port);
};

void PopTranslateSettings::setEnableAuth(bool enable) {
    all_.enable_auth = enable;
    settings_.setValue("enable_auth", enable);
    emit enableAuthChanged(enable);
    qDebug() << tr("Settings: %1 proxy auth")
                    .arg(enable ? tr("Enable") : tr("Disable"));
};

void PopTranslateSettings::setProxyUsername(QString username) {
    all_.proxy_username = username;
    settings_.setValue("proxy_username", username);
    emit proxyUsernameChanged(username);
    qDebug() << tr("Settings: Change proxy username");
};

void PopTranslateSettings::setProxyPassword(QString password) {
    all_.proxy_password = password;
    settings_.setValue("proxy_password", password);
    emit proxyPasswordChanged(password);
    qDebug() << tr("Settings: Change proxy password");
};

void PopTranslateSettings::setTranslateSelectionShortcut(
    QKeySequence shortcut) {
    all_.translate_selection_shortcut = shortcut;
    settings_.setValue("shortcut_popup_main", shortcut);
    emit translateSelectionShortcutChanged(shortcut);
    qDebug() << tr("Settings: Change translate selection shortcut to %1")
                    .arg(shortcut.toString());
};

void PopTranslateSettings::setOcrShortcut(QKeySequence shortcut) {
    all_.ocr_shortcut = shortcut;
    settings_.setValue("ocr_shortcut", shortcut);
    emit ocrShortcutChanged(shortcut);
    qDebug()
        << tr("Settings: Change ocr shortcut to %1").arg(shortcut.toString());
};

void PopTranslateSettings::setPopupWindowSize(QSize size) {
    all_.popup_window_size = size;
    settings_.setValue("popup_window_size", size);
    emit popupWindowSizeChanged(size);
};

// void PopTranslateSettings::setShowSrcText(bool enable) {
//     all_.show_src_text = enable;
//     settings_.setValue("show_src_text", enable);
//     emit showSrcTextChanged(enable);
// };

void PopTranslateSettings::setDictionaries(
    QVector<DictionaryInfo> dictionaries_info) {
    all_.dictionaries_info = dictionaries_info;
    settings_.setValue("dictionaries_info",
                       QVariant::fromValue(dictionaries_info));
    emit dictionariesChanged(dictionaries_info);
    qDebug() << tr(
        "Settings: Change dictionaries_info");  // TODO: add dictionaries info
};

void PopTranslateSettings::setOcrLanguages(QStringList ocr_languages) {
    all_.ocr_languages = ocr_languages;
    settings_.setValue("ocr_languages", ocr_languages);
    emit ocrLanguagesChanged(ocr_languages);
    qDebug() << tr("Settings: Change ocr languages : %1")
                    .arg(ocr_languages.join(" "));
};

void PopTranslateSettings::setMonitorClipboard(bool enable) {
    all_.monitor_clipboard = enable;
    settings_.setValue("monitor_clipboard", enable);
    emit monitorClipboardChanged(enable);
    qDebug() << tr("Settings: %1 monitor clipboard")
                    .arg(enable ? tr("Enable") : tr("Disable"));
};