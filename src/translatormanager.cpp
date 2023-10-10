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
    translators_.append(translator);
}

void TranslatorManager::setSourceLanguage(
    QOnlineTranslator::Language language) {
    source_language_ = language;
}

void TranslatorManager::setTargetLanguage(
    QOnlineTranslator::Language language) {
    target_language_ = language;
}

void TranslatorManager::translate(const QString& text) {
    source_text_ = text;
    for (AbstractTranslator* t : translators_) {
        translateCoro(t, text);
    }
}

QCoro::Task<void> TranslatorManager::translateCoro(AbstractTranslator* t,
                                                   const QString text) {
    t->setTargetLanguage(target_language_);
    t->setSourceLanguage(source_language_);
    auto result_generator = t->translate(text);

    auto result_it = co_await result_generator.begin();
    while (result_it != result_generator.end()) {
        auto result = *result_it;
        if (!result.content.isEmpty() && text == source_text_) {
            emit resultAvailable(result);
        }
        co_await ++result_it;
    }
}