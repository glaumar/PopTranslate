#include "translatormanager.h"

#include "poptranslatesettings.h"

TranslatorManager::TranslatorManager(QObject* parent)
    : QObject(parent),
      source_language_(QOnlineTranslator::Auto),
      target_language_(
          PopTranslateSettings::instance().activeTargetLanguage()) {
    connect(&PopTranslateSettings::instance(),
            &PopTranslateSettings::activeTargetLanguageChanged,
            this,
            &TranslatorManager::setTargetLanguage);
}

void TranslatorManager::addTranslator(AbstractTranslator* translator) {
    translator->setTargetLanguage(
        PopTranslateSettings::instance().activeTargetLanguage());
    translators_.append(qMakePair(true, translator));
    connect(translator,
            &AbstractTranslator::resultAvailable,
            [this](AbstractTranslator::Result result) {
                // filter outdated results
                if (result.source_text == source_text_) {
                    emit resultAvailable(result);
                }
            });

    int i = translators_.size() - 1;
    connect(translator, &AbstractTranslator::finished, [this, i](QString text) {
        auto& [available, t] = translators_[i];
        if (text == source_text_) {
            available = true;
        } else {
            t->setTargetLanguage(target_language_);
            t->setSourceLanguage(source_language_);
            t->translate(source_text_);
        }
    });
}

void TranslatorManager::setSourceLanguage(
    QOnlineTranslator::Language language) {
    // for (auto [available, t] : translators_) {
    //     t->setSourceLanguage(language);
    // }
    source_language_ = language;
}

void TranslatorManager::setTargetLanguage(
    QOnlineTranslator::Language language) {
    // for (auto [available, t] : translators_) {
    //     t->setTargetLanguage(language);
    // }
    target_language_ = language;
}

void TranslatorManager::translate(const QString& text) {
    source_text_ = text;
    for (auto& [available, t] : translators_) {
        // translate with all available translators
        // unavailable translators will translate after the finished signal
        if (available) {
            available = false;
            t->setTargetLanguage(target_language_);
            t->setSourceLanguage(source_language_);
            t->translate(text);
        }
    }
}

void TranslatorManager ::abortAll() {
    for (auto [available, t] : translators_) {
        t->abort();
    }
}
