#include "phrasevalidator.h"
#include "utils.h"

#include <QChar>
#include <QString>
#include <QRegExp>
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
            error_info = QString::fromLocal8Bit("短语名称\n长度超过15个字符！");
            return false;
        }

        if(!IsEnglishChar(abbr.at(0)))
        {
            error_info = QString::fromLocal8Bit("短语名称\n未以英文字母开头！");
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
