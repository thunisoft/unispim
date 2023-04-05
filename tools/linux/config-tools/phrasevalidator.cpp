#include "phrasevalidator.h"
#include "../public/utils.h"

#include <QChar>
#include <QString>
PhraseValidator::PhraseValidator()
{

}

bool PhraseValidator::IsValidAbbr(const QString& abbr, QString& error_info)
{
    /*  短语名称规则 :
        首字母必须是 英文字母
        名字中可以包含数字,但在之前必须有下划线
        且字符只能是英文字母 下划线 数字 分号(:)
        最后一个字符可以是 : 且只能是最后一位*/
        if(abbr.length() > 15)
        {
            error_info = QString::fromLocal8Bit("短语名称长度超过15个字符！");
            return false;
        }

        bool has_underline = false;
        if(!IsEnglishChar(abbr.at(0)))
        {
            error_info = QString::fromLocal8Bit("短语名称未以英文字母开头！");
            return false;
        }
        for(int i = 1; i < abbr.length() - 1; i++)
        {
            if(IsEnglishChar(abbr.at(i)))
                continue;
            if(abbr.at(i) == QChar('_'))
            {
                has_underline = true;
                continue;
            }
            if(abbr.at(i).isDigit() && has_underline)
                continue;

            error_info = QString::fromLocal8Bit("短语名称中含有非法字符！");
            return false;
        }

        QChar last_char = abbr.at(abbr.length() - 1);
        if(!IsEnglishChar(last_char) && !(last_char.isDigit() && has_underline)
                && last_char != QChar('_') && last_char != QChar(':'))
        {
            error_info = QString::fromLocal8Bit("短语名称最后一个字符不合法！");
            return false;
        }

        return true;
}

bool PhraseValidator::IsEnglishChar(const QChar &char_)
{
      return ((char_ >= QChar('a') && char_ <= QChar('z')) || (char_ >= QChar('A') && char_ <= QChar('Z')));
}

bool PhraseValidator::IsValidContent(const QString& content)
{
    if(content.length() > 100)
        return false;
    return  true;
}
