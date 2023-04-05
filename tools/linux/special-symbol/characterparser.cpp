#include "characterparser.h"
#include <QFile>
#include <QTextStream>
#include <QDir>
CharacterParser::CharacterParser(QObject *parent) : QObject(parent)
{
#ifdef USE_FCITX
    m_symbolFile_addr = QDir::toNativeSeparators("/opt/apps/com.thunisoft.input/files/symbols.txt");
#else
    m_symbolFile_addr = QDir::toNativeSeparators("/usr/share/ibus-huayupy/symbols.txt");
#endif
}

bool CharacterParser::ParserFile()
{
    QFile file(":/symbols.txt");
    bool isOpenOK = file.open(QIODevice::ReadOnly);
    if(!isOpenOK)
    {
        return false;
    }

    //get tab name
    QTextStream input(&file);
    QString fullContent = input.readAll();
    QRegExp matchExp("\\[[/_\u4e00-\u9fa5]+\\]");
    matchExp.setMinimal(true);
    int pos = 0;
    m_symbol_vector.clear();
    QVector<QPair<QString, int> > tabIndex;
    while((pos = matchExp.indexIn(fullContent,pos)) != -1)
    {
        QString tabName = matchExp.cap(0);
        pos += matchExp.matchedLength();
        QPair<QString, int> currentPair = qMakePair(tabName, pos);
        tabIndex.push_back(currentPair);
    }

    int size = tabIndex.size();
    for(int index=0; index<size; ++index)
    {
        QPair<QString,int> currentPair = tabIndex.at(index);
        if(index < (size-1))
        {
            QPair<QString,int> nextPair = tabIndex.at(index+1);
            int startPos = currentPair.second + 2;
            int endPos = nextPair.second - nextPair.first.length() - 5;
            int num = endPos - startPos + 1;
            QString content = fullContent.mid(startPos,num);
            QPair<QString,QString> insertPair = qMakePair(currentPair.first,content);
            m_symbol_vector.push_back(insertPair);
        }
        else
        {
            int startPos = currentPair.second + 2;
            int length = fullContent.length() - startPos - 2;
            QString content = fullContent.mid(startPos,length);
            QPair<QString,QString> insertPair = qMakePair(currentPair.first,content);
            m_symbol_vector.push_back(insertPair);
        }
    }
    file.close();
    return true;
}

void CharacterParser::SetSymbolFileAddr(const QString &fileAddr)
{
    m_symbolFile_addr = fileAddr;
}

QVector<QPair<QString,QString> > CharacterParser::GetResult()
{
    return m_symbol_vector;
}
