#include "settingwindow.h"

#include <QMetaEnum>
#include <QSharedPointer>

#include "ui_settingwindow.h"

SettingWindow::SettingWindow(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::SettingWindow),
      settings_(new QSettings("PopTranslate", "PopTranslate", this)),
      target_languages_({
          default_.target_language_1,
          default_.target_language_2,
          default_.target_language_3,
      }) {
    ui->setupUi(this);

    target_languages_combobox_ = {
        ui->translate_lang_combobox_1,
        ui->translate_lang_combobox_2,
        ui->translate_lang_combobox_3,
    };

    // load settings
    initSettings();

    // init ui
    initTranslateEngineComboBox();
    initTargetLanguageComboBox();
    initFont();
}

void SettingWindow::initTranslateEngineComboBox() {
    const QMetaEnum engines = QMetaEnum::fromType<QOnlineTranslator::Engine>();
    // add all translate engines to translate engine combobox
    for (int i = 0; i < engines.keyCount(); i++) {
        ui->translate_engine_combobox->addItem(engines.key(i),
                                               engines.value(i));
    }

    // emit signal when translate engine changed
    connect(
        ui->translate_engine_combobox,
        QOverload<int>::of(&QComboBox::currentIndexChanged),
        [this](int index) {
            // get enum (QOnlineTranslator::Engine) value and key
            int value_int =
                ui->translate_engine_combobox->itemData(index).toInt();
            auto value = static_cast<QOnlineTranslator::Engine>(value_int);
            auto key = DefaultSettings::enumValueToKey(value);
            settings_->setValue("translate_engine", key);
            emit translateEngineChanged(value);

            qDebug() << tr("Settings: Change translate_engine to %1").arg(key);
        });

    // set current translate engine
    auto current_engine =
        strKeyToEnumValue<QOnlineTranslator::Engine>("translate_engine");
    int index = static_cast<int>(current_engine);
    ui->translate_engine_combobox->setCurrentIndex(index);
}

void SettingWindow::initTargetLanguageComboBox() {
    const QMetaEnum languages =
        QMetaEnum::fromType<QOnlineTranslator::Language>();

    // add all languages to all target language comboboxes
    for (int i = 0; i < languages.keyCount(); i++) {
        if (languages.value(i) == QOnlineTranslator::Auto ||
            languages.value(i) == QOnlineTranslator::NoLanguage)
            continue;

        for (auto combobox : target_languages_combobox_) {
            combobox->addItem(languages.key(i), languages.value(i));
        }
    }

    // emit signal when target language changed
    for (int i = 0; i < target_languages_combobox_.size(); i++) {
        connect(
            target_languages_combobox_[i],
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            [this, i](int index) {
                // get enum (QOnlineTranslator::Language) value and key
                int value_int =
                    target_languages_combobox_[i]->itemData(index).toInt();
                auto value =
                    static_cast<QOnlineTranslator::Language>(value_int);
                auto key = default_.enumValueToKey<>(value);

                settings_->setValue(QString("target_language_%1").arg(i + 1),
                                    key);

                target_languages_[i] = value;
                emit targetLanguagesChanged(target_languages_);

                qDebug() << QString("Settings: Change target_language_%1 to %2")
                                .arg(i + 1)
                                .arg(key);
            });
    }

    // set current target languages
    for (int i = 0; i < target_languages_combobox_.size(); i++) {
        target_languages_[i] = strKeyToEnumValue<QOnlineTranslator::Language>(
            QString("target_language_%1").arg(i + 1));

        // QOnlineTranslator::Auto(0) and QOnlineTranslator::NoLanguage(-1) are
        // not in combobox. combobox index start with 0, language enum start
        // with 1.
        int index = static_cast<int>(target_languages_[i]) - 1;
        target_languages_combobox_[i]->setCurrentIndex(index >= 0 ? index : 0);
    }
}

void SettingWindow::initSettings() {
    if (settings_->status() != QSettings::NoError) {
        qWarning() << "Settings: Failed to load settings";
    }
    // If the key is not defined, use the default value
    setValueIfIsNull("translate_engine", default_.translate_engine_to_str());
    setValueIfIsNull("target_language_1", default_.target_language_1_to_str());
    setValueIfIsNull("target_language_2", default_.target_language_2_to_str());
    setValueIfIsNull("target_language_3", default_.target_language_3_to_str());
    setValueIfIsNull("font", default_.font);
    // emit settingLoaded();
}

void SettingWindow::initFont() {
    auto font = settings_->value("font").value<QFont>();

    // emit signal when font changed
    connect(ui->font_kfontrequester,
            &KFontRequester::fontSelected,
            [this](const QFont &font) {
                settings_->setValue("font", font);
                emit fontChanged(font);
                qDebug() << tr("Settings: Change font to %1 %2 %3")
                                .arg(font.family(), font.styleName())
                                .arg(font.pointSize());
            });

    ui->font_kfontrequester->setFont(font);
}

SettingWindow::~SettingWindow() {
    settings_->sync();
    delete ui;
}
