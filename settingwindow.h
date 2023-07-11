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

    static const DefaultSettings kDefaultSettings;

   signals:
    void settingLoaded();
    void translateEngineChanged(QOnlineTranslator::Engine engine);
    void targetLanguageChanged(int index, QOnlineTranslator::Language language);

   private:
    void initSettings();
    void initTranslateEngineComboBox();
    void initTargetLanguageComboBox();

    template <class T>
    inline void setValueIfIsNull(const QString &key, const T value) {
        if (!settings_->contains(key)) {
            settings_->setValue(key, value);
            qDebug()
                << QString("Set %1 to default value: %2").arg(key).arg(value);
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
    inline T strKeyToEnumValue(const QString &setting_key) {
        auto emta_enum = QMetaEnum::fromType<T>();
        int value_int = emta_enum.keyToValue(
            settings_->value(setting_key).toString().toUtf8().data());
        return static_cast<T>(value_int);
    }

    Ui::SettingWindow *ui;
    QSettings *settings_;
    QVector<QComboBox *> translate_lang_combobox_;
    DefaultSettings default_;
    ;
};

#endif  // SettingWindow_H