#ifndef SettingWindow_H
#define SettingWindow_H

#include <QComboBox>
#include <QNetworkProxy>
#include <QSettings>
#include <QWidget>

#include "defaultsettings.h"
#include "qonlinetranslator.h"

namespace Ui {
class SettingWindow;
}

class SettingWindow : public QWidget {
    Q_OBJECT

   public:
    explicit SettingWindow(QWidget *parent = nullptr);
    ~SettingWindow();

    inline QVector<QOnlineTranslator::Language> targetLanguages() const {
        return target_languages_;
    };

    inline QOnlineTranslator::Engine translateEngine() const {
        return strKeyToEnumValue<QOnlineTranslator::Engine>("translate_engine");
    };

    inline QFont font() const {
        return settings_->value("font").value<QFont>();
    };

    inline qreal opacity() const {
        return settings_->value("opacity").toReal();
    };

    inline bool isEnableBlur() const {
        return settings_->value("enable_blur").toBool();
    };

    inline QNetworkProxy proxy() const { return proxy_; };

    inline QKeySequence shortcuts() const {
        return settings_->value("shortcut_popup_main").value<QKeySequence>();
    };

    inline QSize popupWindowSize() const {
        return settings_->value("popup_window_size").value<QSize>();
    };

    inline void setPopupWindowSize(QSize size) {
        settings_->setValue("popup_window_size", size);
    };

    inline bool showSrcText() const {
        return settings_->value("show_src_text").toBool();
    }

    inline void setShowSrcText(bool enable) {
        settings_->setValue("show_src_text", enable);
    }

    inline QStringList dictionaries() const {
        return settings_->value("dictionaries").toStringList();
    }

   signals:
    void translateEngineChanged(QOnlineTranslator::Engine engine);
    void targetLanguagesChanged(QVector<QOnlineTranslator::Language> languages);
    void fontChanged(const QFont &font);
    void opacityChanged(qreal opacity);
    void triggerBlurEffect(bool enable);
    void shortcutChanged(const QKeySequence &  seq);
    void dictionariesChanged(const QStringList &dictionaries);
    // void dictionaryRemoved(const QString &dictionary);

   private:
    void initSettings();
    void initTranslateEngineComboBox();
    void initTargetLanguageComboBox();
    void initFont();
    void initOpacityAndBlur();
    void initProxy();
    void initShortcut();
    void initDictionaries();

    template <class T>
    inline void setValueIfIsNull(const QString &key, const T value) {
        if (!settings_->contains(key)) {
            settings_->setValue(key, value);
        }
    }

    // for human readable, enum value (translate_engine and target_langeuage)
    // saved as string in configuration file. this function convert the string
    // to enum value after configuration file loaded.
    //
    // for example, if translate_engine is "Google" in configuration file, then
    // call this function with "translate_engine", it will return
    // QOnlineTranslator::Google
    template <class T>
    inline T strKeyToEnumValue(const QString &setting_key) const {
        auto emta_enum = QMetaEnum::fromType<T>();
        int value_int = emta_enum.keyToValue(
            settings_->value(setting_key).toString().toUtf8().data());
        return static_cast<T>(value_int);
    }

    Ui::SettingWindow *ui;
    QSettings *settings_;
    QVector<QComboBox *> target_languages_combobox_;
    QVector<QOnlineTranslator::Language> target_languages_;
    const DefaultSettings default_;
    QNetworkProxy proxy_;
};

#endif  // SettingWindow_H