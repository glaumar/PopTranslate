#include "settingwindow.h"

#include <QFileDialog>
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
    initOpacityAndBlur();
    initProxy();
    initShortcut();
    initDictionaries();
}

SettingWindow::~SettingWindow() {
    settings_->sync();
    delete ui;
}

void SettingWindow::initTranslateEngineComboBox() {
    const QMetaEnum engines = QMetaEnum::fromType<QOnlineTranslator::Engine>();
    // add all translate engines to translate engine combobox
    for (int i = 0; i < engines.keyCount(); i++) {
        // TODO: Add support for LibreTranslate and Lingva
        // Disable LibreTranslate and Lingva
        if (engines.value(i) == QOnlineTranslator::Engine::LibreTranslate ||
            engines.value(i) == QOnlineTranslator::Engine::Lingva) {
            continue;
        }
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
        connect(target_languages_combobox_[i],
                QOverload<int>::of(&QComboBox::currentIndexChanged),
                [this, i](int index) {
                    // get enum (QOnlineTranslator::Language) value and key
                    int value_int =
                        target_languages_combobox_[i]->itemData(index).toInt();
                    auto value =
                        static_cast<QOnlineTranslator::Language>(value_int);
                    auto key = default_.enumValueToKey<>(value);

                    settings_->setValue(
                        QString("target_language_%1").arg(i + 1),
                        key);

                    target_languages_[i] = value;
                    emit targetLanguagesChanged(target_languages_);

                    qDebug() << tr("Settings: Change target_language_%1 to %2")
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
    setValueIfIsNull("opacity", default_.opacity);
    setValueIfIsNull("enable_blur", default_.enable_blur);
    setValueIfIsNull("enable_proxy", default_.enable_proxy);
    setValueIfIsNull("proxy_hostname", default_.proxy_hostname);
    setValueIfIsNull("proxy_port", default_.proxy_port);
    setValueIfIsNull("enable_auth", default_.enable_auth);
    setValueIfIsNull("proxy_username", default_.proxy_username);
    setValueIfIsNull("proxy_password", default_.proxy_password);
    setValueIfIsNull("shortcut_popup_main", default_.shortcut_popup_main);
    // setValueIfIsNull("shortcut_popup_alt", default_.shortcut_popup_alt);
    setValueIfIsNull("popup_window_size", default_.popup_window_size);
    setValueIfIsNull("show_src_text", default_.show_src_text);
    setValueIfIsNull("dictionaries", default_.dictionaries);
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

void SettingWindow::initOpacityAndBlur() {
    auto opacity = settings_->value("opacity").value<qreal>();
    auto enable_blur = settings_->value("enable_blur").value<bool>();

    // emit signal when opacity changed
    connect(ui->opacity_slider, &QSlider::valueChanged, [this](int value) {
        qreal opacity = static_cast<qreal>(value) / 100;
        settings_->setValue("opacity", opacity);
        emit opacityChanged(opacity);
        qDebug() << tr("Settings: Change opacity to %1").arg(opacity);
    });

    // emit signal when blur effect changed
    connect(ui->blur_checkbox, &QCheckBox::stateChanged, [this](int state) {
        auto enable_blur = state == Qt::Checked;
        settings_->setValue("enable_blur", enable_blur);
        emit triggerBlurEffect(enable_blur);
        qDebug() << tr("Settings: Change blur effect to %1").arg(enable_blur);
    });

    ui->opacity_slider->setValue(static_cast<int>(opacity * 100));
    ui->blur_checkbox->setChecked(enable_blur);
}

void SettingWindow::initProxy() {
    auto enable_proxy = settings_->value("enable_proxy").toBool();
    auto proxy_hostname = settings_->value("proxy_hostname").toString();
    auto proxy_port = settings_->value("proxy_port").value<quint16>();
    auto enable_auth = settings_->value("enable_auth").toBool();
    auto proxy_username = settings_->value("proxy_username").toString();
    auto proxy_password = settings_->value("proxy_password").toString();

    // set proxy for application
    if (enable_proxy) {
        if (enable_auth) {
            proxy_ = QNetworkProxy(QNetworkProxy::HttpProxy,
                                   proxy_hostname,
                                   proxy_port);
        } else {
            proxy_ = QNetworkProxy(QNetworkProxy::HttpProxy,
                                   proxy_hostname,
                                   proxy_port,
                                   proxy_username,
                                   proxy_password);
        }
        QNetworkProxy::setApplicationProxy(proxy_);

        qDebug() << tr("Settings: Change proxy to %1:%2")
                        .arg(proxy_.hostName())
                        .arg(proxy_.port());
    } else {
        QNetworkProxy::setApplicationProxy(QNetworkProxy::NoProxy);
    }

    // set proxy settings ui
    ui->http_proxy_checkbox->setChecked(enable_proxy);
    ui->proxy_host_lineedit->setText(proxy_hostname);
    ui->proxy_port_spinbox->setValue(proxy_port);
    ui->auth_checkbox->setChecked(enable_auth);
    ui->username_lineedit->setText(proxy_username);
    ui->password_lineedit->setText(proxy_password);

    // enable/disable proxy settings ui
    ui->proxy_host_lineedit->setEnabled(enable_proxy);
    ui->proxy_port_spinbox->setEnabled(enable_proxy);
    ui->auth_checkbox->setEnabled(enable_proxy);
    ui->username_lineedit->setEnabled(enable_auth && enable_proxy);
    ui->password_lineedit->setEnabled(enable_auth && enable_proxy);

    // Enable/Disable proxy
    connect(
        ui->http_proxy_checkbox,
        &QCheckBox::stateChanged,
        [this](int state) {
            auto enable_proxy = state == Qt::Checked;
            settings_->setValue("enable_proxy", enable_proxy);
            if (enable_proxy) {
                QNetworkProxy::setApplicationProxy(proxy_);
            } else {
                QNetworkProxy::setApplicationProxy(QNetworkProxy::NoProxy);
            }

            auto enable_auth = ui->auth_checkbox->isChecked();
            ui->proxy_host_lineedit->setEnabled(enable_proxy);
            ui->proxy_port_spinbox->setEnabled(enable_proxy);
            ui->auth_checkbox->setEnabled(enable_proxy);
            ui->username_lineedit->setEnabled(enable_auth && enable_proxy);
            ui->password_lineedit->setEnabled(enable_auth && enable_proxy);

            qDebug() << tr("Settings: %1 proxy %2:%3")
                            .arg(enable_proxy ? tr("Enable") : tr("Disable"))
                            .arg(proxy_.hostName())
                            .arg(proxy_.port());
        });

    // Enable/Disable proxy auth
    connect(ui->auth_checkbox, &QCheckBox::stateChanged, [this](int state) {
        auto enable_auth = state == Qt::Checked;

        if (enable_auth) {
            proxy_.setUser(ui->username_lineedit->text());
            proxy_.setPassword(ui->password_lineedit->text());
        } else {
            proxy_.setUser(QString());
            proxy_.setPassword(QString());
        }

        ui->username_lineedit->setEnabled(enable_auth);
        ui->password_lineedit->setEnabled(enable_auth);

        settings_->setValue("enable_auth", enable_auth);

        qDebug() << tr("Settings: %1 proxy auth")
                        .arg(enable_auth ? tr("Enable") : tr("Disable"));
    });

    // Set slot for change proxy setting
    connect(
        ui->proxy_host_lineedit,
        &QLineEdit::textChanged,
        [this](const QString &text) {
            proxy_.setHostName(text);
            QNetworkProxy::setApplicationProxy(proxy_);
            settings_->setValue("proxy_hostname", text);
            qDebug() << tr("Settings: Change proxy hostname to %1").arg(text);
        });
    connect(ui->proxy_port_spinbox,
            QOverload<int>::of(&QSpinBox::valueChanged),
            [this](int value) {
                proxy_.setPort(value);
                QNetworkProxy::setApplicationProxy(proxy_);
                settings_->setValue("proxy_port", value);
                qDebug() << tr("Settings: Change proxy port to %1").arg(value);
            });
    connect(
        ui->username_lineedit,
        &QLineEdit::textChanged,
        [this](const QString &text) {
            proxy_.setUser(text);
            QNetworkProxy::setApplicationProxy(proxy_);
            settings_->setValue("proxy_username", text);
            qDebug() << tr("Settings: Change proxy username to %1").arg(text);
        });
    connect(
        ui->password_lineedit,
        &QLineEdit::textChanged,
        [this](const QString &text) {
            proxy_.setPassword(text);
            QNetworkProxy::setApplicationProxy(proxy_);
            settings_->setValue("proxy_password", text);
            qDebug() << tr("Settings: Change proxy password to %1").arg(text);
        });
}

void SettingWindow::initShortcut() {
    // emit signal when shortcut changed
    connect(ui->shortcut_kkeysequencewidget,
            &KKeySequenceWidget::keySequenceChanged,
            [this](const QKeySequence &seq) {
                settings_->setValue("shortcut_popup_main", seq);
                qDebug() << tr("Settings: Change main shortcut to %1")
                                .arg(seq.toString());
                emit shortcutChanged(seq);
            });

    ui->shortcut_kkeysequencewidget->setKeySequence(this->shortcuts());
}

void SettingWindow::initDictionaries() {
    auto *add_button = ui->dictionary_keditlistwidget->addButton();
    add_button->setEnabled(true);
    ui->dictionary_keditlistwidget->lineEdit()->setVisible(false);

    // add dictionary files using filedialog
    connect(add_button, &QPushButton::pressed, [this] {
        static auto *dialog =
            new QFileDialog(this, tr("Select Dictionary Files"));
        dialog->setFileMode(QFileDialog::ExistingFiles);
        dialog->setNameFilter(tr("MDict Files (*.mdx)"));
        dialog->setDirectory(QDir::homePath());
        dialog->exec();
        if (!dialog->selectedFiles().isEmpty()) {
            ui->dictionary_keditlistwidget->insertStringList(
                dialog->selectedFiles());
        };
        emit ui->dictionary_keditlistwidget->changed();
    });

    connect(ui->dictionary_keditlistwidget, &KEditListWidget::changed, [this] {
        settings_->setValue("dictionaries",
                            ui->dictionary_keditlistwidget->items());
        qDebug() << tr("Settings: Change dictionaries : %1")
                        .arg(ui->dictionary_keditlistwidget->items().join(" "));
        emit dictionariesChanged(this->dictionaries());
        ui->dictionary_keditlistwidget->addButton()->setEnabled(true);
    });

    // connect(ui->dictionary_keditlistwidget,
    //         &KEditListWidget::removed,
    //         [this](const QString &item) {
    //             settings_->setValue("dictionaries",
    //                                 ui->dictionary_keditlistwidget->items());
    //             qDebug() << tr("Settings: Remove dictionaries : %1").arg(item);
    //             emit dictionaryRemoved(item);
    //             ui->dictionary_keditlistwidget->addButton()->setEnabled(true);
    //         });

    auto dictionaries = this->dictionaries();
    if (!dictionaries.isEmpty()) {
        ui->dictionary_keditlistwidget->setItems(this->dictionaries());
    }
}
