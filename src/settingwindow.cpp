#include "settingwindow.h"

#include <QFileDialog>
#include <QMetaEnum>
#include <QSharedPointer>
#include <QToolButton>

#include "langcode2name.h"
#include "ocr.h"
#include "ui_settingwindow.h"

SettingWindow::SettingWindow(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::SettingWindow),
      target_languages_(PopTranslateSettings::instance().targetLanguages()) {
    ui->setupUi(this);

    target_languages_combobox_ = {
        ui->translate_lang_combobox_1,
        ui->translate_lang_combobox_2,
        ui->translate_lang_combobox_3,
    };

    initTranslateEngineComboBox();
    initTargetLanguageComboBox();
    initFont();
    initOpacityAndBlur();
    initAutoCopyTranslation();
    initAutoSpeak();
    // initShowSrcText();
    initProxy();
    initShortcut();
    initDictionaries();
    initOcrLanguages();
}

SettingWindow::~SettingWindow() { delete ui; }

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

    // set current translate engine
    auto current_engine = PopTranslateSettings::instance().translateEngine();
    int index = static_cast<int>(current_engine);
    ui->translate_engine_combobox->setCurrentIndex(index);

    // emit signal when translate engine changed
    connect(ui->translate_engine_combobox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            [this](int index) {
                // get enum (QOnlineTranslator::Engine) value and key
                int value_int =
                    ui->translate_engine_combobox->itemData(index).toInt();
                auto value = static_cast<QOnlineTranslator::Engine>(value_int);

                PopTranslateSettings::instance().setTranslateEngine(value);
            });

    connect(&PopTranslateSettings::instance(),
            &PopTranslateSettings::translateEngineChanged,
            [this](QOnlineTranslator::Engine engine) {
                int index = static_cast<int>(engine);
                ui->translate_engine_combobox->setCurrentIndex(index);
            });
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

    // set current target languages
    target_languages_ = PopTranslateSettings::instance().targetLanguages();
    for (int i = 0; i < target_languages_combobox_.size(); i++) {
        // QOnlineTranslator::Auto(0) and QOnlineTranslator::NoLanguage(-1) are
        // not in combobox. combobox index start with 0, language enum start
        // with 1.
        int index = static_cast<int>(target_languages_[i]) - 1;
        target_languages_combobox_[i]->setCurrentIndex(index >= 0 ? index : 0);
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

                    target_languages_[i] = value;
                    PopTranslateSettings::instance().setTargetLanguages(
                        target_languages_);
                });
    }
}

void SettingWindow::initFont() {
    ui->font_kfontrequester->setFont(PopTranslateSettings::instance().font());

    // emit signal when font changed
    connect(ui->font_kfontrequester,
            &KFontRequester::fontSelected,
            [this](const QFont &font) {
                PopTranslateSettings::instance().setFont(font);
            });
}

void SettingWindow::initOpacityAndBlur() {
    ui->opacity_slider->setValue(
        static_cast<int>(PopTranslateSettings::instance().opacity() * 100));
    ui->blur_checkbox->setChecked(
        PopTranslateSettings::instance().isEnableBlur());

    // emit signal when opacity changed
    connect(ui->opacity_slider, &QSlider::valueChanged, [this](int value) {
        qreal opacity = static_cast<qreal>(value) / 100;
        PopTranslateSettings::instance().setOpacity(opacity);
    });

    // emit signal when blur effect changed
    connect(ui->blur_checkbox, &QCheckBox::stateChanged, [this](int state) {
        auto enable_blur = state == Qt::Checked;
        PopTranslateSettings::instance().setEnableBlur(enable_blur);
    });
}

void SettingWindow::initAutoCopyTranslation() {
    ui->auto_copy_translation_checkbox->setChecked(
        PopTranslateSettings::instance().isEnableAutoCopyTranslation());

    connect(ui->auto_copy_translation_checkbox,
            &QCheckBox::stateChanged,
            [this](int state) {
                auto enable = state == Qt::Checked;
                PopTranslateSettings::instance().setEnableAutoCopyTranslation(
                    enable);
            });
}

void SettingWindow::initAutoSpeak() {
    ui->auto_speak_checkbox->setChecked(
        PopTranslateSettings::instance().isEnableAutoSpeak());

    connect(ui->auto_speak_checkbox,
            &QCheckBox::stateChanged,
            [this](int state) {
                auto enable = state == Qt::Checked;
                PopTranslateSettings::instance().setEnableAutoSpeak(enable);
            });
}

// void SettingWindow::initShowSrcText() {
//     ui->show_src_text_checkbox->setChecked(
//         PopTranslateSettings::instance().showSrcText());

//     connect(ui->show_src_text_checkbox,
//             &QCheckBox::stateChanged,
//             [this](int state) {
//                 auto enable = state == Qt::Checked;
//                 PopTranslateSettings::instance().setShowSrcText(enable);
//             });
// }

void SettingWindow::initProxy() {
    auto enable_proxy = PopTranslateSettings::instance().isEnableProxy();
    auto proxy_hostname = PopTranslateSettings::instance().proxyHostname();
    auto proxy_port = PopTranslateSettings::instance().proxyPort();
    auto enable_auth = PopTranslateSettings::instance().isEnableAuth();
    auto proxy_username = PopTranslateSettings::instance().proxyUsername();
    auto proxy_password = PopTranslateSettings::instance().proxyPassword();

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
    connect(ui->http_proxy_checkbox,
            &QCheckBox::stateChanged,
            [this](int state) {
                auto enable_proxy = state == Qt::Checked;
                PopTranslateSettings::instance().setEnableProxy(enable_proxy);
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

        PopTranslateSettings::instance().setEnableAuth(enable_auth);
    });

    // Set slot for change proxy setting
    connect(ui->proxy_host_lineedit,
            &QLineEdit::textChanged,
            [this](const QString &text) {
                proxy_.setHostName(text);
                QNetworkProxy::setApplicationProxy(proxy_);
                PopTranslateSettings::instance().setProxyHostname(text);
            });
    connect(ui->proxy_port_spinbox,
            QOverload<int>::of(&QSpinBox::valueChanged),
            [this](int value) {
                proxy_.setPort(value);
                QNetworkProxy::setApplicationProxy(proxy_);
                PopTranslateSettings::instance().setProxyPort(value);
            });
    connect(ui->username_lineedit,
            &QLineEdit::textChanged,
            [this](const QString &text) {
                proxy_.setUser(text);
                QNetworkProxy::setApplicationProxy(proxy_);
                PopTranslateSettings::instance().setProxyUsername(text);
            });
    connect(ui->password_lineedit,
            &QLineEdit::textChanged,
            [this](const QString &text) {
                proxy_.setPassword(text);
                QNetworkProxy::setApplicationProxy(proxy_);
                PopTranslateSettings::instance().setProxyPassword(text);
            });
}

void SettingWindow::initShortcut() {
    ui->translate_selection_kkeysequencewidget->setKeySequence(
        PopTranslateSettings::instance().TranslateSelectionShortcut());
    connect(
        ui->translate_selection_kkeysequencewidget,
        &KKeySequenceWidget::keySequenceChanged,
        [this](const QKeySequence &seq) {
            PopTranslateSettings::instance().setTranslateSelectionShortcut(seq);
        });

    ui->ocr_kkeysequencewidget->setKeySequence(
        PopTranslateSettings::instance().OcrShortcut());
    connect(ui->ocr_kkeysequencewidget,
            &KKeySequenceWidget::keySequenceChanged,
            [this](const QKeySequence &seq) {
                PopTranslateSettings::instance().setOcrShortcut(seq);
            });
}

void SettingWindow::initDictionaries() {
    // ui->dict_listwidget->setDragDropMode(QAbstractItemView::InternalMove);
    // ui->dict_listwidget->setMovement(QListView::Snap);

    for (const auto &dict_info :
         PopTranslateSettings::instance().dictionaries()) {
        addDictionaryItem(dict_info);
    }

    connect(ui->add_pushbutton, &QPushButton::clicked, [this]() {
        static auto *dialog = new QFileDialog(this,
                                              tr("Select Dictionary Files"),
                                              QDir::homePath());
        dialog->setFileMode(QFileDialog::ExistingFiles);
        dialog->setNameFilter(tr("MDict Files (*.mdx)"));
        dialog->exec();
        if (!dialog->selectedFiles().isEmpty()) {
            for (auto f : dialog->selectedFiles()) {
                addDictionaryItem(DictionaryInfo{f, QOnlineTranslator::Auto});
            }
        };
    });
}

void SettingWindow::initOcrLanguages() {
    auto layout = ui->ocr_languages_layout;
    layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    // checkbox disable until ocr languages available (setAvailableOcrLanguages)
    ocr_languages_enable_ =
        addOcrLanguageToUi(PopTranslateSettings::instance().ocrLanguages(),
                           true,
                           false);

    Ocr ocr;
    ocr.init("eng", "", {});
    auto availabel_ocr_languages = ocr.availableLanguages();
    // Enable all ocr languages that load from settings
    for (const auto &language_code : ocr_languages_enable_.keys()) {
        if (availabel_ocr_languages.removeAll(language_code) > 0) {
            ocr_languages_enable_[language_code]->setEnabled(true);
        }
    }
    addOcrLanguageToUi(availabel_ocr_languages);
}

QMap<QString, QCheckBox *> SettingWindow::addOcrLanguageToUi(
    QStringList ocr_languages,
    bool is_checked,
    bool is_enabled) {
    QMap<QString, QCheckBox *> result;
    for (const auto language_code : ocr_languages) {
        QString language_name = LangCode2Name(language_code);
        auto checkbox =
            new QCheckBox(QString("%1 (%2)").arg(language_name, language_code),
                          this);
        checkbox->setChecked(is_checked);
        checkbox->setEnabled(is_enabled);

        result[language_code] = checkbox;
        connect(
            checkbox,
            &QCheckBox::stateChanged,
            [this, language_code](int state) {
                QStringList orc_languages =
                    PopTranslateSettings::instance().ocrLanguages();
                if (state == Qt::Checked) {
                    orc_languages.append(language_code);
                } else {
                    orc_languages.removeOne(language_code);
                }
                PopTranslateSettings::instance().setOcrLanguages(orc_languages);
            });

        ui->ocr_languages_layout->addWidget(checkbox);
    }

    return result;
}

void SettingWindow::addDictionaryItem(const DictionaryInfo &dict_info) {
    QListWidgetItem *item = new QListWidgetItem(ui->dict_listwidget);
    item->setData(Qt::UserRole, QVariant::fromValue(dict_info));
    QWidget *item_widget = newDictionaryItemWidget(dict_info, item);

    item->setSizeHint(item_widget->sizeHint());

    ui->dict_listwidget->addItem(item);
    ui->dict_listwidget->setItemWidget(item, item_widget);
    ui->dict_listwidget->setMinimumWidth(
        ui->dict_listwidget->sizeHintForColumn(0));

    PopTranslateSettings::instance().setDictionaries(getAllDictInfo());
}

QWidget *SettingWindow::newDictionaryItemWidget(const DictionaryInfo &dict_info,
                                                QListWidgetItem *item) {
    QWidget *item_widget = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(item_widget);

    QLabel *dictname =
        new QLabel(QFileInfo(dict_info.filename).baseName(), item_widget);
    dictname->setToolTip(dict_info.filename);

    QComboBox *combobox = new QComboBox(item_widget);
    combobox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);

    combobox->setToolTip(tr("Target Language"));

    // add all languages to all target language comboboxes
    const QMetaEnum languages =
        QMetaEnum::fromType<QOnlineTranslator::Language>();
    for (int i = 0; i < languages.keyCount(); i++) {
        if (languages.value(i) == QOnlineTranslator::NoLanguage) continue;
        if (languages.value(i) == QOnlineTranslator::Auto) {
            combobox->addItem(tr("All Languages"), languages.value(i));
        } else {
            combobox->addItem(languages.key(i), languages.value(i));
        }
    }

    combobox->setCurrentIndex(static_cast<int>(dict_info.target_language));

    connect(
        combobox,
        QOverload<int>::of(&QComboBox::currentIndexChanged),
        [this, item](int index) {
            auto dict_info = item->data(Qt::UserRole).value<DictionaryInfo>();
            dict_info.target_language =
                static_cast<QOnlineTranslator::Language>(index);
            item->setData(Qt::UserRole, QVariant::fromValue(dict_info));

            PopTranslateSettings::instance().setDictionaries(getAllDictInfo());
        });

    // button to move dictionary up
    QToolButton *up = new QToolButton(item_widget);
    up->setIcon(QIcon::fromTheme("go-up"));
    connect(up, &QToolButton::clicked, [this, item, item_widget]() {
        int r = ui->dict_listwidget->row(item);
        if (r > 0) {
            ui->dict_listwidget->takeItem(r);
            // delete item_widget;

            ui->dict_listwidget->insertItem(r - 1, item);
            auto dict_info = item->data(Qt::UserRole).value<DictionaryInfo>();
            ui->dict_listwidget->setItemWidget(
                item,
                newDictionaryItemWidget(dict_info, item));
            ui->dict_listwidget->setCurrentItem(item);

            PopTranslateSettings::instance().setDictionaries(getAllDictInfo());
        }
    });

    // button to move dictionary down
    QToolButton *down = new QToolButton(item_widget);
    down->setIcon(QIcon::fromTheme("go-down"));
    connect(down, &QToolButton::clicked, [this, item, item_widget]() {
        int r = ui->dict_listwidget->row(item);
        if (r < ui->dict_listwidget->count() - 1) {
            ui->dict_listwidget->takeItem(r);
            // delete item_widget;

            ui->dict_listwidget->insertItem(r + 1, item);
            auto dict_info = item->data(Qt::UserRole).value<DictionaryInfo>();
            ui->dict_listwidget->setItemWidget(
                item,
                newDictionaryItemWidget(dict_info, item));
            ui->dict_listwidget->setCurrentItem(item);

            PopTranslateSettings::instance().setDictionaries(getAllDictInfo());
        }
    });

    // button to remove dictionary
    QToolButton *remove = new QToolButton(item_widget);
    remove->setIcon(QIcon::fromTheme("delete"));
    connect(remove, &QToolButton::clicked, [this, item, item_widget]() {
        int r = ui->dict_listwidget->row(item);
        delete ui->dict_listwidget->takeItem(r);
        // delete item_widget;
        PopTranslateSettings::instance().setDictionaries(getAllDictInfo());

        ui->dict_listwidget->setMinimumWidth(
            ui->dict_listwidget->sizeHintForColumn(0));
    });

    layout->addWidget(combobox);
    layout->addWidget(dictname);
    layout->addWidget(up);
    layout->addWidget(down);
    layout->addWidget(remove);
    item_widget->setLayout(layout);

    return item_widget;
}

QVector<DictionaryInfo> SettingWindow::getAllDictInfo() const {
    QVector<DictionaryInfo> all_dict_info;
    for (int i = 0; i < ui->dict_listwidget->count(); ++i) {
        auto item = ui->dict_listwidget->item(i);
        all_dict_info.append(item->data(Qt::UserRole).value<DictionaryInfo>());
    }
    return all_dict_info;
}