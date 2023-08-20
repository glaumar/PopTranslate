#pragma once

#include <QFont>
#include <QKeySequence>
#include <QMetaEnum>
#include <QObject>
#include <QOnlineTranslator>
#include <QSettings>
#include <QSize>
#include <QStringList>
#include <QVector>

#include "allsettings.h"
#include "poptranslate.h"

class PopTranslateSettings : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(PopTranslateSettings)

   public:
    static PopTranslateSettings& instance() {
        static PopTranslateSettings inst;
        return inst;
    }

    void load();

    inline QOnlineTranslator::Engine translateEngine() const {
        return all_.translate_engine;
    };
    inline QString translateEngineStr() const {
        return translateEngineStr(all_.translate_engine);
    };
    static inline QString translateEngineStr(QOnlineTranslator::Engine engine) {
        return enumValueToKey(engine);
    };
    static inline QString targetLanguageStr(QOnlineTranslator::Language lang) {
        return enumValueToKey(lang);
    };
    inline QStringList targetLanguagesStr() {
        return targetLanguagesStr(all_.targetLanguages);
    };
    static inline QStringList targetLanguagesStr(
        QVector<QOnlineTranslator::Language> languages) {
        QStringList languages_list;
        for (auto lang : languages) {
            languages_list.append(enumValueToKey(lang));
        }
        return languages_list;
    };
    static inline QVector<QOnlineTranslator::Language>
    stringListToTargetLanguages(QStringList languagelist) {
        QVector<QOnlineTranslator::Language> languages;
        for (auto lang : languagelist) {
            languages.append(enumKeyToValue<QOnlineTranslator::Language>(
                lang.toStdString().c_str()));
        }
        return languages;
    };
    inline QVector<QOnlineTranslator::Language> targetLanguages() const {
        return all_.targetLanguages;
    };
    inline QFont font() const { return all_.font; };
    inline qreal opacity() const { return all_.opacity; };
    inline bool isEnableBlur() const { return all_.enable_blur; };
    inline bool isEnableAutoCopyTranslation() const {
        return all_.enable_auto_copy_translation;
    };
    inline bool isEnableProxy() const { return all_.enable_proxy; };
    inline QString proxyHostname() const { return all_.proxy_hostname; };
    inline quint16 proxyPort() const { return all_.proxy_port; };
    inline bool isEnableAuth() const { return all_.enable_auth; };
    inline QString proxyUsername() const { return all_.proxy_username; };
    inline QString proxyPassword() const { return all_.proxy_password; };
    inline QKeySequence TranslateSelectionShortcut() const {
        return all_.translate_selection_shortcut;
    };
    inline QKeySequence OcrShortcut() const { return all_.ocr_shortcut; };
    inline QSize popupWindowSize() const { return all_.popup_window_size; };
    inline bool showSrcText() const { return all_.show_src_text; };
    inline QStringList dictionaries() const { return all_.dictionaries; };
    inline QStringList ocrLanguages() const { return all_.ocr_languages; };

    void setTranslateEngine(QOnlineTranslator::Engine engine);
    void setTargetLanguages(QVector<QOnlineTranslator::Language> languages);
    void setFont(QFont font);
    void setOpacity(qreal opacity);
    void setEnableBlur(bool enable);
    void setEnableAutoCopyTranslation(bool enable);
    void setEnableProxy(bool enable);
    void setProxyHostname(QString hostname);
    void setProxyPort(quint16 port);
    void setEnableAuth(bool enable);
    void setProxyUsername(QString username);
    void setProxyPassword(QString password);
    void setTranslateSelectionShortcut(QKeySequence shortcut);
    void setOcrShortcut(QKeySequence shortcut);
    void setPopupWindowSize(QSize size);
    void setShowSrcText(bool enable);
    void setDictionaries(QStringList dictionaries);
    void setOcrLanguages(QStringList ocr_languages);

   signals:
    void translateEngineChanged(QOnlineTranslator::Engine engine);
    void fontChanged(QFont font);
    void targetLanguagesChanged(QVector<QOnlineTranslator::Language> languages);
    void opacityChanged(qreal opacity);
    void enableBlurChanged(bool enable);
    void enableAutoCopyTranslationChanged(bool enable);
    void enableProxyChanged(bool enable);
    void proxyHostnameChanged(QString hostname);
    void proxyPortChanged(quint16 port);
    void enableAuthChanged(bool enable);
    void proxyUsernameChanged(QString username);
    void proxyPasswordChanged(QString password);
    void translateSelectionShortcutChanged(QKeySequence shortcut);
    void ocrShortcutChanged(QKeySequence shortcut);
    void popupWindowSizeChanged(QSize size);
    void showSrcTextChanged(bool enable);
    void dictionariesChanged(QStringList dictionaries);
    void ocrLanguagesChanged(QStringList ocr_languages);

   private:
    PopTranslateSettings();
    ~PopTranslateSettings() { settings_.sync(); }
    // PopTranslateSettings(const PopTranslateSettings&) = delete;
    // PopTranslateSettings& operator=(const PopTranslateSettings&) = delete;

    template <class T>
    static inline T enumKeyToValue(const char* key) {
        int value = QMetaEnum::fromType<T>().keyToValue(key);
        return static_cast<T>(value);
    }

    template <class T>
    static inline const char* enumValueToKey(const T value) {
        return QMetaEnum::fromType<T>().valueToKey(value);
    }

    QSettings settings_;
    AllSettings all_;
};
