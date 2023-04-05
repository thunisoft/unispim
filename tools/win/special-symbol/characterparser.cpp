#include "characterparser.h"
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QApplication>
#include <QTextCodec>
#include "charhandler.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif


CharacterParser* CharacterParser::m_instance = NULL;

CharacterParser *CharacterParser::Instance()
{
    if(m_instance == NULL)
    {
        m_instance = new CharacterParser();
    }
    return m_instance;
}


CharacterParser::CharacterParser(QObject *parent) : QObject(parent)
{
  QString applicate_path = QDir::toNativeSeparators(QApplication::applicationDirPath());
  m_symbolFile_addr = QString(":/symbols.txt");
  ParserSymbolDescribeFile(":/data/symbol-describe.txt");
}

CharacterParser::~CharacterParser()
{
    delete m_instance;
    m_instance = NULL;
}

bool CharacterParser::ParserFile()
{
    bool isFileExist = QFile::exists(m_symbolFile_addr);
    if(!isFileExist)
    {
        return false;
    }
    QFile file(m_symbolFile_addr);
    bool isOpenOK = file.open(QIODevice::ReadOnly);
    if(!isOpenOK)
    {
        return false;
    }

    //get tab name
    QTextStream input(&file);
    input.setCodec(QTextCodec::codecForName("UTF-8"));
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

bool CharacterParser::OuputSymbolToCSV(QString filePath)
{
    QFile file(filePath);
    if(!file.open(QIODevice::ReadWrite | QIODevice::Truncate))
    {
        return false;
    }

    QTextStream outputStream(&file);
    QStringList headerList;
    headerList << "一级标题" << "二级标题" << "符号" << "中文名称";
    int groupNums = m_symbol_vector.size();
    outputStream << headerList.join("\t") << "\r\n";
    for(int index=0; index<groupNums; ++index)
    {
        QString groupNameFullStr = m_symbol_vector.at(index).first;
        QString groupName = groupNameFullStr.mid(1,groupNameFullStr.length()-2);
        QPair<QString,QString> tempPair = m_symbol_vector.at(index);
        QString groupSymbol = tempPair.second;

        QStringList symbolList = groupSymbol.split("\r\n\r\n");
        int childGroupSize = symbolList.size();

        for(int childIndex=0; childIndex<childGroupSize; ++childIndex)
        {
           CharHandler* handler = NULL;
           if((groupName == "英文音符")||(groupName == "数字单位")||(groupName == "中文部首"))
           {

               handler = new DoubleCharHandler(symbolList.at(childIndex));
           }
           else
           {
               handler = new SingleCharHandler(symbolList.at(childIndex));

           }
           if(handler == NULL)
           {
               continue;
           }
           QVector<QVector<QString> > singleSymbolVector = handler->GetResult();
           for(int singleIndex=0;singleIndex<singleSymbolVector.size(); ++singleIndex)
           {
                QVector<QString> lineVector = singleSymbolVector.at(singleIndex);
                for(int lineIndex = 0; lineIndex<lineVector.size(); ++lineIndex)
                {
                    QStringList lineContentList;
                    QString childGroup = QString("group%1").arg(childIndex+1);
                    lineContentList << groupName << childGroup << lineVector[lineIndex];
                    outputStream << lineContentList.join("\t") << "\t" << "\r\n";
                }
           }

           delete  handler;
           handler = NULL;
        }

    }

    file.close();

    return true;
}

QString CharacterParser::GetSymbolDescribe(QString titlename, QString symbol)
{

    if(titlename.isEmpty())
    {
        QList<QString> keyList = m_symbolDescribeMap.keys();
        for(QString key: keyList)
        {
            QVector<SYMBOL_INFO> symbolVector = m_symbolDescribeMap[key];
            for(SYMBOL_INFO indexinfo : symbolVector)
            {
                if(indexinfo.symbolName == symbol)
                {
                    return indexinfo.symbolDesctribe;
                }
            }
        }
    }

    QVector<SYMBOL_INFO> symbolVector = m_symbolDescribeMap[titlename];
    for(SYMBOL_INFO indexinfo : symbolVector)
    {
        if(indexinfo.symbolName == symbol)
        {
            return indexinfo.symbolDesctribe;
        }
    }
    return QString("特殊符号");
}

QVector<QString> CharacterParser::GetSearchResult(QString searchIndex)
{
    QVector<QString> result;
    if(searchIndex.isEmpty())
    {
        return result;
    }

    QList<QString> keyList = m_symbolDescribeMap.keys();
    for(QString key: keyList)
    {
        QVector<SYMBOL_INFO> symbolVector = m_symbolDescribeMap[key];
        for(SYMBOL_INFO indexinfo : symbolVector)
        {
            if(indexinfo.symbolDesctribe.contains(searchIndex))
            {
                result.push_back(indexinfo.symbolName);
            }
        }
    }
    return result;

}

bool CharacterParser::ParserSymbolDescribeFile(QString sourcePath)
{
    if(!QFile::exists(sourcePath))
    {
        return false;
    }
    QFile source_file(sourcePath);

    if(source_file.open(QIODevice::ReadOnly))
    {
        QTextStream inputStream(&source_file);
        QString content = inputStream.readAll();
        QStringList lines = content.split("\r\n");
        for(QString line : lines)
        {
            QStringList SymbolList = line.split("\t");
            if(SymbolList.size() != 4)
            {
                continue;
            }
            SYMBOL_INFO tempSymbolInfo;
            tempSymbolInfo.symbolName = SymbolList.at(2);
            tempSymbolInfo.symbolDesctribe = SymbolList.at(3);
            m_symbolDescribeMap[SymbolList.at(0)].push_back(tempSymbolInfo);

        }
        source_file.close();
        return true;
    }
    return false;

}
