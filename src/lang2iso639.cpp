#include "lang2iso639.h"

#include <QMap>
#include <QObject>
#include <QString>

static const QMap<QOnlineTranslator::Language, QString> KISO639 = {
    {QOnlineTranslator::NoLanguage, QString("")},
    {QOnlineTranslator::Auto, QString("AUTO")},
    {QOnlineTranslator::Afrikaans, QString("AF")},
    {QOnlineTranslator::Albanian, QString("SQ")},
    {QOnlineTranslator::Amharic, QString("AM")},
    {QOnlineTranslator::Arabic, QString("AR")},
    {QOnlineTranslator::Armenian, QString("HY")},
    {QOnlineTranslator::Azerbaijani, QString("AZ")},
    {QOnlineTranslator::Bashkir, QString("BA")},
    {QOnlineTranslator::Basque, QString("EU")},
    {QOnlineTranslator::Belarusian, QString("BE")},
    {QOnlineTranslator::Bengali, QString("BN")},
    {QOnlineTranslator::Bosnian, QString("BS")},
    {QOnlineTranslator::Bulgarian, QString("BG")},
    {QOnlineTranslator::Cantonese, QString("YUE")},
    {QOnlineTranslator::Catalan, QString("CA")},
    {QOnlineTranslator::Cebuano, QString("CEB")},
    {QOnlineTranslator::Chichewa, QString("NY")},
    {QOnlineTranslator::Corsican, QString("CO")},
    {QOnlineTranslator::Croatian, QString("HR")},
    {QOnlineTranslator::Czech, QString("CS")},
    {QOnlineTranslator::Danish, QString("DA")},
    {QOnlineTranslator::Dutch, QString("NL")},
    {QOnlineTranslator::English, QString("EN")},
    {QOnlineTranslator::Esperanto, QString("EO")},
    {QOnlineTranslator::Estonian, QString("ET")},
    {QOnlineTranslator::Fijian, QString("FJ")},
    {QOnlineTranslator::Filipino, QString("FIL")},
    {QOnlineTranslator::Finnish, QString("FI")},
    {QOnlineTranslator::French, QString("FR")},
    {QOnlineTranslator::Frisian, QString("FY")},
    {QOnlineTranslator::Galician, QString("GL")},
    {QOnlineTranslator::Georgian, QString("KA")},
    {QOnlineTranslator::German, QString("DE")},
    {QOnlineTranslator::Greek, QString("EL")},
    {QOnlineTranslator::Gujarati, QString("GU")},
    {QOnlineTranslator::HaitianCreole, QString("HT")},
    {QOnlineTranslator::Hausa, QString("HA")},
    {QOnlineTranslator::Hawaiian, QString("HAW")},
    {QOnlineTranslator::Hebrew, QString("HE")},
    {QOnlineTranslator::HillMari, QString("MRJ")},
    {QOnlineTranslator::Hindi, QString("HI")},
    {QOnlineTranslator::Hmong, QString("HMN")},
    {QOnlineTranslator::Hungarian, QString("HU")},
    {QOnlineTranslator::Icelandic, QString("IS")},
    {QOnlineTranslator::Igbo, QString("IG")},
    {QOnlineTranslator::Indonesian, QString("ID")},
    {QOnlineTranslator::Irish, QString("GA")},
    {QOnlineTranslator::Italian, QString("IT")},
    {QOnlineTranslator::Japanese, QString("JA")},
    {QOnlineTranslator::Javanese, QString("JV")},
    {QOnlineTranslator::Kannada, QString("KN")},
    {QOnlineTranslator::Kazakh, QString("KK")},
    {QOnlineTranslator::Khmer, QString("KM")},
    {QOnlineTranslator::Kinyarwanda, QString("RW")},
    {QOnlineTranslator::Klingon, QString("TLH")},
    {QOnlineTranslator::KlingonPlqaD, QString("TLH")},  // maybe wrong
    {QOnlineTranslator::Korean, QString("KO")},
    {QOnlineTranslator::Kurdish, QString("KU")},
    {QOnlineTranslator::Kyrgyz, QString("KY")},
    {QOnlineTranslator::Lao, QString("LO")},
    {QOnlineTranslator::Latin, QString("LA")},
    {QOnlineTranslator::Latvian, QString("LV")},
    {QOnlineTranslator::LevantineArabic, QString("APC")},
    {QOnlineTranslator::Lithuanian, QString("LT")},
    {QOnlineTranslator::Luxembourgish, QString("LB")},
    {QOnlineTranslator::Macedonian, QString("MK")},
    {QOnlineTranslator::Malagasy, QString("MG")},
    {QOnlineTranslator::Malay, QString("MS")},
    {QOnlineTranslator::Malayalam, QString("ML")},
    {QOnlineTranslator::Maltese, QString("MT")},
    {QOnlineTranslator::Maori, QString("MI")},
    {QOnlineTranslator::Marathi, QString("MR")},
    {QOnlineTranslator::Mari, QString("CHM")},
    {QOnlineTranslator::Mongolian, QString("MN")},
    {QOnlineTranslator::Myanmar, QString("MY")},
    {QOnlineTranslator::Nepali, QString("NE")},
    {QOnlineTranslator::Norwegian, QString("NO")},
    {QOnlineTranslator::Oriya, QString("OR")},
    {QOnlineTranslator::Papiamento, QString("PAP")},
    {QOnlineTranslator::Pashto, QString("PS")},
    {QOnlineTranslator::Persian, QString("FA")},
    {QOnlineTranslator::Polish, QString("PL")},
    {QOnlineTranslator::Portuguese, QString("PT")},
    {QOnlineTranslator::Punjabi, QString("PA")},
    {QOnlineTranslator::QueretaroOtomi, QString("OTQ")},
    {QOnlineTranslator::Romanian, QString("RO")},
    {QOnlineTranslator::Russian, QString("RU")},
    {QOnlineTranslator::Samoan, QString("SM")},
    {QOnlineTranslator::ScotsGaelic, QString("GD")},
    {QOnlineTranslator::SerbianCyrillic, QString("SR")},
    {QOnlineTranslator::SerbianLatin, QString("SR")},
    {QOnlineTranslator::Sesotho, QString("ST")},
    {QOnlineTranslator::Shona, QString("SN")},
    {QOnlineTranslator::SimplifiedChinese, QString("ZH")},
    {QOnlineTranslator::Sindhi, QString("SD")},
    {QOnlineTranslator::Sinhala, QString("SI")},
    {QOnlineTranslator::Slovak, QString("SK")},
    {QOnlineTranslator::Slovenian, QString("SL")},
    {QOnlineTranslator::Somali, QString("SO")},
    {QOnlineTranslator::Spanish, QString("ES")},
    {QOnlineTranslator::Sundanese, QString("SU")},
    {QOnlineTranslator::Swahili, QString("SW")},
    {QOnlineTranslator::Swedish, QString("SV")},
    {QOnlineTranslator::Tagalog, QString("TL")},
    {QOnlineTranslator::Tahitian, QString("TY")},
    {QOnlineTranslator::Tajik, QString("TG")},
    {QOnlineTranslator::Tamil, QString("TA")},
    {QOnlineTranslator::Tatar, QString("TT")},
    {QOnlineTranslator::Telugu, QString("TE")},
    {QOnlineTranslator::Thai, QString("TH")},
    {QOnlineTranslator::Tongan, QString("TO")},
    {QOnlineTranslator::TraditionalChinese,
     QString("ZH_TC")},  // TC is not ISO 639 code
    {QOnlineTranslator::Turkish, QString("TR")},
    {QOnlineTranslator::Turkmen, QString("TK")},
    {QOnlineTranslator::Udmurt, QString("UDM")},
    {QOnlineTranslator::Uighur, QString("UG")},
    {QOnlineTranslator::Ukrainian, QString("UK")},
    {QOnlineTranslator::Urdu, QString("UR")},
    {QOnlineTranslator::Uzbek, QString("UZ")},
    {QOnlineTranslator::Vietnamese, QString("VI")},
    {QOnlineTranslator::Welsh, QString("CY")},
    {QOnlineTranslator::Xhosa, QString("XH")},
    {QOnlineTranslator::Yiddish, QString("YI")},
    {QOnlineTranslator::Yoruba, QString("YO")},
    {QOnlineTranslator::YucatecMaya, QString("YUA")},
    {QOnlineTranslator::Zulu, QString("ZU")},
};

QString Lang2ISO639(QOnlineTranslator::Language lang) { return KISO639[lang]; }