#include "phrasemodel.h"
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QTextCodec>
#include <stdio.h>
#include <Windows.h>
#include "config.h"

PhraseModel* PhraseModel::m_instance = NULL;

PhraseModel* PhraseModel::Instance()
{
    if(!m_instance)
        m_instance = new PhraseModel();
    return m_instance;
}

PhraseModel::PhraseModel()
{
    Init();
}

void PhraseModel::Init()
{
    LoadPhraseFromFile();
}

void PhraseModel::LoadPhraseFromFile()
{
    m_unparsed_phrases.clear();
    m_phrase.clear();
    ParsePhraseFile();
    ParsePhraseToCell(m_unparsed_phrases);
}

void PhraseModel::ParsePhraseFile()
{
    //获取文件的路径
    QFile phrase_file(Config::Instance()->CustomPhraseFilePath());
    if(!phrase_file.exists())
        return;
    phrase_file.open(QFile::ReadOnly);
    m_unparsed_phrases.clear();
    QString one_phrase_data;
    //要考虑一行有多个'='的情况
    QTextCodec *codec = QTextCodec::codecForName("UTF-16LE");
    QTextStream  read_stream(&phrase_file);
    read_stream.setCodec(codec);
    while(!read_stream.atEnd())
    {
        QString line = read_stream.readLine();
        if(line.count(QChar('=')) > 1) //一行中有多个'='，跳过解析
        {
            continue;
        }
        if(line.isEmpty() && one_phrase_data.isEmpty()) //空行的情况，直接跳过
        {
            continue;
        }
        if(line.contains(QChar('=')) && one_phrase_data.contains(QChar('=')))
        {
            m_unparsed_phrases << QString(one_phrase_data);
            one_phrase_data.clear();
            one_phrase_data = line;
        }
        else if(line.contains(QChar('=')) && one_phrase_data.isEmpty())//这一行只有内容
        {
            one_phrase_data.append(line);
        }
        else
        {
            one_phrase_data.append('\n').append(line);
        }
    }
    if(!one_phrase_data.isEmpty())
        m_unparsed_phrases << QString(one_phrase_data); //解析到的最后一个短语放入list
    phrase_file.close();
}

void PhraseModel::ParsePhraseToCell(const QStringList& phrases)
{
    m_phrase.clear();
    QString abbr_and_position;
    QString abbr;
    int position = 0;
    QString content;
    int line_count = -1;
    foreach(QString line, phrases)
    {
        line_count++;
        int index_of_equal_sign = line.indexOf('=');
        abbr_and_position = line.left(index_of_equal_sign);
        content = line.right(line.length() - index_of_equal_sign - 1);
        if(content.endsWith('\n'))
        {
            content.remove(content.length() - 1, 1);
        }
        if(content.isEmpty())
            continue;
        if(!ParsePhraseAbbrAndPosition(abbr_and_position, abbr, position))
            continue;
        PHRASE phrase;
        phrase.abbr = abbr;
        phrase.position = position;
        phrase.content = content;
        phrase.state = NONE;
        MakePhraseSuitStandard(phrase);
        m_phrase.insert(line_count, phrase);
    }
}

bool PhraseModel::ParsePhraseAbbrAndPosition(const QString& abbr_and_position, QString& abbr, int& position)
{
    int position_of_left_kuohao = abbr_and_position.indexOf('[');
    if(position_of_left_kuohao == -1)
    {
        abbr = abbr_and_position;
        position = 0;
    }
    else
    {
        abbr = abbr_and_position.left(position_of_left_kuohao);
        position = GetPhraseDisplayPosition(abbr_and_position.right(abbr_and_position.length() - position_of_left_kuohao));
    }
    if(!VerifyAbbr(abbr))
        return false;
    return true;
}

int PhraseModel::GetPhraseDisplayPosition(const QString& str_to_analysis)
{
    if(!str_to_analysis.contains('[') || !str_to_analysis.contains(']') || str_to_analysis.indexOf('[') > str_to_analysis.indexOf(']'))
        return 0;
    QString position_str = str_to_analysis.mid(str_to_analysis.indexOf('[') + 1, str_to_analysis.indexOf(']') - str_to_analysis.indexOf('[') - 1);
    bool success = false;
    int position = position_str.toInt(&success);
    if(success)
        return position;
    return 0;
}

bool PhraseModel::VerifyAbbr(const QString& abbr)
{
    //自定义短语名称的规则，只允许输入英文字符和;且分号不能在开头结尾
    QRegExp reg("^[a-zA-Z][a-zA-Z;]{0,20}[a-zA-Z;]{0,1}");
    return reg.exactMatch(abbr);
}

bool PhraseModel::IsEnglishChar(const QChar& char_)
{
    return ((char_ >= QChar('a') && char_ <= QChar('z')) || (char_ >= QChar('A') && char_ <= QChar('Z')));
}

void PhraseModel::MakePhraseSuitStandard(PHRASE &phrase)
{
    int all_character_length = phrase.content.length();
    if(all_character_length > 2048)
    {
        QString new_content = phrase.content.left(2048);
        phrase.content = new_content;
        phrase.state = MODIFIED;
    }
    if(IsRepeat(phrase.abbr, phrase.content))
    {
        phrase.state = REMOVED;
    }
}

void PhraseModel::SavePhraseToRealFile()
{
    QFile file(Config::Instance()->CustomPhraseFilePath());
    if(!file.exists())
    {
        QFileInfo file_info(Config::Instance()->CustomPhraseFilePath());
        Utils::MakeDir(file_info.absoluteDir().absolutePath());
    }

    file.open(QFile::WriteOnly);
    QTextStream out_stream(&file);
    QTextCodec *codec = QTextCodec::codecForName("UTF-16LE");
    out_stream.setCodec(codec);
    out_stream.setGenerateByteOrderMark(true);
    foreach(PHRASE phrase, m_phrase)
    {
        if(phrase.state == REMOVED)
            continue;
        out_stream << TransformPhraseToLine(phrase) << "\r\n";
    }
    out_stream.flush();
    file.close();
}


QMap<int, PHRASE> PhraseModel::GetParsedPhrase()
{
    return m_phrase;
}

void PhraseModel::SetParsedPhrase(QMap<int,PHRASE> newPhrase)
{
    m_phrase.clear();
    m_phrase = newPhrase;
}

QString PhraseModel::TransformPhraseToLine(const PHRASE& phrase)
{
    QString line_data;
    line_data.append(phrase.abbr);
    if(phrase.position >=1 && phrase.position <= 9)
    {
        line_data.append("[").append(QString::number(phrase.position).append("]"));
    }
    line_data.append("=");
    line_data.append(phrase.content);    
    return line_data;
}

bool PhraseModel::IsRepeat(const QString& abbr, const QString& content, const int phrase_index)
{
    QMap<int, PHRASE>::iterator it = m_phrase.begin();
    for(; it != m_phrase.end(); it++)
    {
        if(phrase_index == it.key())
            continue;
        if(it.value().abbr.compare(abbr) == 0
                && it.value().content.compare(content) == 0 && (it.value().state != REMOVED))
            return true;
    }
    return false;
}

wchar_t* PhraseModel::UTF8ToUnicodeBegin(const char* str)
{
    char* sz = new char[strlen(str) + 1];
    strcpy(sz, str);
    int wLen = MultiByteToWideChar(CP_UTF8, 0, sz, -1, NULL, 0);
    wchar_t* wBuf = new wchar_t[wLen + 1];
    MultiByteToWideChar(CP_UTF8, 0, sz, -1, wBuf, wLen);
    delete[] sz;
    return wBuf;
}

void PhraseModel::UTF8ToUnicodeEnd(wchar_t* wstr_chaged)
{
    if (wstr_chaged)
    {
        delete[] wstr_chaged;
        wstr_chaged = NULL;
    }
}
