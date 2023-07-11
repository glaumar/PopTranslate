#ifndef DEFAULTSETTINGS_H
#define DEFAULTSETTINGS_H

#include <QOnlineTranslator>
#include <QMetaEnum>

// get variable name as string, void(Variable) force compiler to check if variable exists
// #define MACRO_VARIABLE_TO_STRING(Variable) (void(Variable),#Variable)

class DefaultSettings {
   public:
    DefaultSettings() {
        translate_engine = QOnlineTranslator::Engine::Google;
        target_language_1 = QOnlineTranslator::Language::SimplifiedChinese;
        target_language_2 = QOnlineTranslator::Language::English;
        target_language_3 = QOnlineTranslator::Language::Japanese;
    }

    QOnlineTranslator::Engine translate_engine;
    QOnlineTranslator::Language target_language_1;
    QOnlineTranslator::Language target_language_2;
    QOnlineTranslator::Language target_language_3;

    inline const char* translate_engine_to_str() const {
        return enumValueToKey(translate_engine);
    }

    inline const char* target_language_1_to_str() const {
        return enumValueToKey(target_language_1);
    }

    inline const char* target_language_2_to_str() const {
        return enumValueToKey(target_language_2);
    }

    inline const char* target_language_3_to_str() const {
        return enumValueToKey(target_language_3);
    }

   private:
    template <class T>
    inline const char* enumValueToKey(const T value) const {
        return QMetaEnum::fromType<T>().valueToKey(value);
    }
};

#endif  // DEFAULTSETTINGS_H