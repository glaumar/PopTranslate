#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QListWidgetItem>
#include <QNetworkProxy>
#include <QWidget>

#include "dictionaryinfo.h"
#include "poptranslatesettings.h"
#include "qonlinetranslator.h"

namespace Ui {
class SettingWindow;
}

class SettingWindow : public QWidget {
    Q_OBJECT

   public:
    explicit SettingWindow(QWidget *parent = nullptr);
    ~SettingWindow();

   private:
    void initTranslateEngineComboBox();
    void initTargetLanguageComboBox();
    void initFont();
    void initOpacityAndBlur();
    void initAutoCopyTranslation();
    void initAutoSpeak();
    // void initShowSrcText();
    void initProxy();
    void initShortcut();
    void initDictionaries();
    void initOcrLanguages();
    QMap<QString, QCheckBox *> addOcrLanguageToUi(QStringList ocr_languages,
                                                  bool is_checked = false,
                                                  bool is_enabled = true);

    void addDictionaryItem(const DictionaryInfo &dict_info);
    QWidget *newDictionaryItemWidget(const DictionaryInfo &dict_info,
                                     QListWidgetItem *item);
    QVector<DictionaryInfo> getAllDictInfo() const;

    Ui::SettingWindow *ui;
    QVector<QComboBox *> target_languages_combobox_;
    QVector<QOnlineTranslator::Language> target_languages_;
    QNetworkProxy proxy_;
    QMap<QString, QCheckBox *> ocr_languages_enable_;
};