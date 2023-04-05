#ifndef PHRASEVALIDATOR_H
#define PHRASEVALIDATOR_H

#include <QString>

class PhraseValidator
{
public:
    PhraseValidator();

    static bool IsValidAbbr(const QString& abbr, QString& error_info);
    static bool IsEnglishChar(const QChar& char_);
    static bool IsValidContent(const QString& content);
};

#endif // PHRASEVALIDATOR_H
