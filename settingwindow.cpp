#include "settingwindow.h"

#include <QMetaEnum>

#include "ui_settingwindow.h"

SettingWindow::SettingWindow(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::SettingWindow),
      settings_(new QSettings("PopTranslate", "PopTranslate", this)) {
    ui->setupUi(this);

    translate_lang_combobox_ = {
        ui->translate_lang_combobox_1,
        ui->translate_lang_combobox_2,
        ui->translate_lang_combobox_3,
    };

    // load settings
    initSettings();

    // init ui
    initTranslateEngineComboBox();
    initTargetLanguageComboBox();
}

void SettingWindow::initTranslateEngineComboBox() {
    const QMetaEnum engines = QMetaEnum::fromType<QOnlineTranslator::Engine>();
    // add all translate engines to translate engine combobox
    for (int i = 0; i < engines.keyCount(); i++) {
        ui->translate_engine_combobox->addItem(engines.key(i),
                                               engines.value(i));
    }

    // emit signal when translate engine changed
    connect(ui->translate_engine_combobox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            [this, engines](int index) {
                int value =
                    ui->translate_engine_combobox->itemData(index).toInt();
                auto key = engines.valueToKey(value);
                settings_->setValue("translate_engine", key);

                emit translateEngineChanged(
                    static_cast<QOnlineTranslator::Engine>(value));

                qDebug() << tr("Change translate_engine to %1").arg(key);
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

        for (auto combobox : translate_lang_combobox_) {
            combobox->addItem(languages.key(i), languages.value(i));
        }
    }

    // emit signal when target language changed
    for (int i = 0; i < translate_lang_combobox_.size(); i++) {
        connect(translate_lang_combobox_[i],
                QOverload<int>::of(&QComboBox::currentIndexChanged),
                [this, i, languages](int index) {
                    int value =
                        translate_lang_combobox_[i]->itemData(index).toInt();
                    auto key = languages.valueToKey(value);
                    settings_->setValue(
                        QString("target_language_%1").arg(i + 1),
                        key);

                    auto lang = static_cast<QOnlineTranslator::Language>(value);
                    emit targetLanguageChanged(i + 1, lang);

                    qDebug() << QString("Change target_language_%1 to %2")
                                    .arg(i + 1)
                                    .arg(key);
                });
    }

    // set current target languages
    for (int i = 0; i < translate_lang_combobox_.size(); i++) {
        auto current_lang = strKeyToEnumValue<QOnlineTranslator::Language>(
            QString("target_language_%1").arg(i + 1));
        // QOnlineTranslator::Auto(0) and QOnlineTranslator::NoLanguage(-1) are not in combobox
        int index = static_cast<int>(current_lang) - 1;
        translate_lang_combobox_[i]->setCurrentIndex(index >= 0 ? index : 0);
    }
}

void SettingWindow::initSettings() {
    if (settings_->status() != QSettings::NoError) {
        qDebug() << "Failed to load settings";
    }
    setValueIfIsNull("translate_engine", default_.translate_engine_to_str());
    setValueIfIsNull("target_language_1", default_.target_language_1_to_str());
    setValueIfIsNull("target_language_2", default_.target_language_2_to_str());
    setValueIfIsNull("target_language_3", default_.target_language_3_to_str());
    emit settingLoaded();
}

SettingWindow::~SettingWindow() {
    settings_->sync();
    delete ui;
}
