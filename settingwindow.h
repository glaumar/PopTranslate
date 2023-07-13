#ifndef SettingWindow_H
#define SettingWindow_H

#include <QComboBox>
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

   signals:
    // void settingLoaded();
    void translateEngineChanged(QOnlineTranslator::Engine engine);
    void targetLanguagesChanged(QVector<QOnlineTranslator::Language> languages);
    void fontChanged(const QFont &font);

   private:
    void initSettings();
    void initTranslateEngineComboBox();
    void initTargetLanguageComboBox();
    void initFont();

    template <class T>
    inline void setValueIfIsNull(const QString &key, const T value) {
        if (!settings_->contains(key)) {
            settings_->setValue(key, value);
            // qDebug() << QString("Settings: Set %1 to default value: %2")
            //                 .arg(key)
            //                 .arg(value);
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
};

#endif  // SettingWindow_H