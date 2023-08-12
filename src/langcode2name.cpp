#include "langcode2name.h"

#include <QMap>
#include <QObject>
#include <QString>

static const QMap<QString, QString> KCodeNameMap = {
    {"afr", QObject::tr("Afrikaans")},
    {"amh", QObject::tr("Amharic")},
    {"ara", QObject::tr("Arabic")},
    {"asm", QObject::tr("Assamese")},
    {"aze", QObject::tr("Azerbaijani")},
    {"aze_cyrl", QObject::tr("Azerbaijani (Cyrillic)")},
    {"bel", QObject::tr("Belarusian")},
    {"ben", QObject::tr("Bengali")},
    {"bod", QObject::tr("Tibetan")},
    {"bos", QObject::tr("Bosnian")},
    {"bre", QObject::tr("Breton")},
    {"bul", QObject::tr("Bulgarian")},
    {"cat", QObject::tr("Catalan")},
    {"ceb", QObject::tr("Cebuano")},
    {"ces", QObject::tr("Czech")},
    {"chi_sim", QObject::tr("Simplified Chinese")},
    {"chi_sim_vert", QObject::tr("Simplified Chinese (vertical)")},
    {"chi_tra", QObject::tr("Traditional Chinese")},
    {"chi_tra_vert", QObject::tr("Traditional Chinese (vertical)")},
    {"chr", QObject::tr("Cherokee")},
    {"cos", QObject::tr("Corsican")},
    {"cym", QObject::tr("Welsh")},
    {"dan", QObject::tr("Danish")},
    {"dan_frak", QObject::tr("Danish (Fraktur)")},
    {"deu", QObject::tr("German")},
    {"deu_frak", QObject::tr("German (Fraktur)")},
    {"div", QObject::tr("Dhivehi")},
    {"dzo", QObject::tr("Dzongkha")},
    {"ell", QObject::tr("Greek")},
    {"eng", QObject::tr("English")},
    {"enm", QObject::tr("Middle English")},
    {"epo", QObject::tr("Esperanto")},
    {"equ", QObject::tr("Quechua")},
    {"est", QObject::tr("Estonian")},
    {"eus", QObject::tr("Basque")},
    {"fao", QObject::tr("Faroese")},
    {"fas", QObject::tr("Persian")},
    {"fil", QObject::tr("Filipino")},
    {"fin", QObject::tr("Finnish")},
    {"fra", QObject::tr("French")},
    {"frk", QObject::tr("Frankish")},
    {"frm", QObject::tr("Middle French")},
    {"fry", QObject::tr("Frisian")},
    {"gla", QObject::tr("Scottish Gaelic")},
    {"gle", QObject::tr("Irish")},
    {"glg", QObject::tr("Galician")},
    {"grc", QObject::tr("Ancient Greek")},
    {"guj", QObject::tr("Gujarati")},
    {"hat", QObject::tr("HaitianCreole")},
    {"heb", QObject::tr("Hebrew")},
    {"hin", QObject::tr("Hindi")},
    {"hrv", QObject::tr("Croatian")},
    {"hun", QObject::tr("Hungarian")},
    {"hye", QObject::tr("Armenian")},
    {"iku", QObject::tr("Inuktitut")},
    {"ind", QObject::tr("Indonesian")},
    {"isl", QObject::tr("Icelandic")},
    {"ita", QObject::tr("Italian")},
    {"ita_old", QObject::tr("Italian (Old)")},
    {"jav", QObject::tr("Javanese")},
    {"jpn", QObject::tr("Japanese")},
    {"jpn_vert", QObject::tr("Japanese (vertical)")},
    {"kan", QObject::tr("Kannada")},
    {"kat", QObject::tr("Georgian")},
    {"kat_old", QObject::tr("Georgian (Old)")},
    {"kaz", QObject::tr("Kazakh")},
    {"khm", QObject::tr("Khmer")},
    {"kir", QObject::tr("Kyrgyz")},
    {"kmr", QObject::tr("Kurdish (Northern)")},
    {"kor", QObject::tr("Korean")},
    {"kor_vert", QObject::tr("Korean (vertical)")},
    {"lao", QObject::tr("Lao")},
    {"lat", QObject::tr("Latin")},
    {"lav", QObject::tr("Latvian")},
    {"lit", QObject::tr("Lithuanian")},
    {"ltz", QObject::tr("Luxembourgish")},
    {"mal", QObject::tr("Malayalam")},
    {"mar", QObject::tr("Marathi")},
    {"mkd", QObject::tr("Macedonian")},
    {"mlt", QObject::tr("Maltese")},
    {"mon", QObject::tr("Mongolian")},
    {"mri", QObject::tr("Maori")},
    {"msa", QObject::tr("Malay")},
    {"mya", QObject::tr("Burmese")},
    {"nep", QObject::tr("Nepali")},
    {"nld", QObject::tr("Dutch")},
    {"nor", QObject::tr("Norwegian")},
    {"oci", QObject::tr("Occitan")},
    {"ori", QObject::tr("Oriya")},
    {"osd", QObject::tr("Orientation and script detection")},
    {"pan", QObject::tr("Punjabi")},
    {"pol", QObject::tr("Polish")},
    {"por", QObject::tr("Portuguese")},
    {"pus", QObject::tr("Pashto")},
    {"que", QObject::tr("Quechua")},
    {"ron", QObject::tr("Romanian")},
    {"rus", QObject::tr("Russian")},
    {"san", QObject::tr("Sanskrit")},
    {"sin", QObject::tr("Sinhalese")},
    {"slk", QObject::tr("Slovak")},
    {"slk_frak", QObject::tr("Slovak (Fraktur)")},
    {"slv", QObject::tr("Slovenian")},
    {"snd", QObject::tr("Sindhi")},
    {"spa", QObject::tr("Spanish")},
    {"spa_old", QObject::tr("Spanish (Old)")},
    {"sqi", QObject::tr("Albanian")},
    {"srp", QObject::tr("Serbian")},
    {"srp_latn", QObject::tr("Serbian (Latin)")},
    {"sun", QObject::tr("Sundanese")},
    {"swa", QObject::tr("Swahili")},
    {"swe", QObject::tr("Swedish")},
    {"syr", QObject::tr("Syriac")},
    {"tam", QObject::tr("Tamil")},
    {"tat", QObject::tr("Tatar")},
    {"tel", QObject::tr("Telugu")},
    {"tgk", QObject::tr("Tajik")},
    {"tgl", QObject::tr("Tagalog")},
    {"tha", QObject::tr("Thai")},
    {"tir", QObject::tr("Tigrinya")},
    {"ton", QObject::tr("Tonga")},
    {"tur", QObject::tr("Turkish")},
    {"uig", QObject::tr("Uighur")},
    {"ukr", QObject::tr("Ukrainian")},
    {"urd", QObject::tr("Urdu")},
    {"uzb", QObject::tr("Uzbek")},
    {"uzb_cyrl", QObject::tr("Uzbek (Cyrillic)")},
    {"vie", QObject::tr("Vietnamese")},
    {"yid", QObject::tr("Yiddish")},
    {"yor", QObject::tr("Yoruba")},
};

QString LangCode2Name(const QString& code) {
    if (KCodeNameMap.contains(code))
        return KCodeNameMap[code];
    else
        return code;
}
