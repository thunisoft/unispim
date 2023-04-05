#ifndef PHRASEMODEL_H
#define PHRASEMODEL_H

#include <QStringList>
#include <QMap>
#include "utils.h"

using namespace OLD_VERSION;
/*
*之前每次做界面样式修改，都会涉及到业务逻辑的代码修改。来回的粘贴代码、移除代码比较麻烦。
*为了解决这个问题，先决定参照java的model-view-controller设计模式。将展现与路基的代码分离。
*不过只做两层model（逻辑处理层）-view(展示层)。model层为展示层提供接口。
*之前的wordlibnetworker已经做了这个这个尝试。
*/

class PhraseModel
{
public:
    static PhraseModel* Instance();
    void LoadPhraseFromFile();
    QMap<int, PHRASE> GetParsedPhrase();
    void SetParsedPhrase(QMap<int,PHRASE> newPhrase);
    void SavePhraseToRealFile();
    bool IsRepeat(const QString& abbr, const QString& content, const int phrase = -1);

private:
    PhraseModel();
    void Init();
    void ParsePhraseFile();
    void ParsePhraseToCell(const QStringList& phrases);
    bool ParsePhraseAbbrAndPosition(const QString& abbr_and_position, QString& abbr, int& position);
    int GetPhraseDisplayPosition(const QString& str_to_analysis);
    bool VerifyAbbr(const QString& abbr);
    bool IsEnglishChar(const QChar& char_);
    void MakePhraseSuitStandard(PHRASE &phrase);
    /*自定义短语分两个文件，一个是用户可见的编辑的自定义短语文件，该文件中可能存在错误短语。
      真实短语文件是输入法引擎读入的真正文件，该文件经过配置工具的过滤，将错误的短语过滤掉，
      这样做的目的是避免一些错误操作，导致引擎在解析自定义短语文件时出错而崩溃。*/
    static PhraseModel *m_instance;
    QString TransformPhraseToLine(const PHRASE& phrase);
    QMap<int, PHRASE> m_phrase;
    QStringList m_unparsed_phrases; //未解析的短语数据

    void UTF8ToUnicodeEnd(wchar_t* wstr_chaged);
    wchar_t* UTF8ToUnicodeBegin(const char* str);
};

#endif // PHRASEMODEL_H
