#include "translatormanager.h"

#include "poptranslatesettings.h"

TranslatorManager::TranslatorManager(QObject* parent) : QObject(parent) {
    connect(&PopTranslateSettings::instance(),
            &PopTranslateSettings::activeTargetLanguageChanged,
            this,
            &TranslatorManager::setTargetLanguage);
}

void TranslatorManager::addTranslator(AbstractTranslator* translator) {
    translator->setTargetLanguage(
        PopTranslateSettings::instance().activeTargetLanguage());
    translators_.append(translator);
    connect(translator,
            &AbstractTranslator::resultAvailable,
            this,
            &TranslatorManager::resultAvailable);

    // TOOD: finished
}

void TranslatorManager::setSourceLanguage(
    QOnlineTranslator::Language language) {
    for (auto t : translators_) {
        t->setSourceLanguage(language);
    }
}

void TranslatorManager::setTargetLanguage(
    QOnlineTranslator::Language language) {
    for (auto t : translators_) {
        t->setTargetLanguage(language);
    }
}

void TranslatorManager::translate(const QString& text) {
    for (auto t : translators_) {
        t->translate(text);
    }
}

void TranslatorManager ::abortAll() {
    for (auto t : translators_) {
        t->abort();
    }
}
