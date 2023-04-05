#include "charhandler.h"
#include <QStringList>


CharHandler::CharHandler(QString inputStr)
{
    m_targetStr = inputStr;
}
CharHandler::~CharHandler()
{

}

SingleCharHandler::SingleCharHandler(QString inputStr):CharHandler(inputStr)
{
    QStringList lineList = m_targetStr.split("\r\n");
    for(int index=0; index<lineList.size(); ++index)
    {
        QVector<QString> charVector;
        QString lineStr = lineList.at(index);
        for(int columnIndex=0; columnIndex<lineStr.length(); ++columnIndex)
        {
            QChar key = lineStr.at(columnIndex);
            charVector.push_back(QString(key));
        }
        m_resultVector.push_back(charVector);
    }
}

SingleCharHandler::~SingleCharHandler()
{

}

QVector<QVector<QString> > SingleCharHandler::GetResult()
{
    return m_resultVector;
}

DoubleCharHandler::DoubleCharHandler(QString inputStr) : CharHandler(inputStr)
{
    QStringList lineList = m_targetStr.split("\r\n");
    for(int index=0; index<lineList.size(); ++index)
    {
        QString lineStr = lineList.at(index);
        QVector<QString> charVector;
        if(lineStr.contains(','))
        {
            QStringList childStrList = lineStr.split(',');
            int childStrSize = childStrList.size();
            for(int columnIndex=0; columnIndex<childStrSize; ++columnIndex)
            {
                QString key = childStrList.at(columnIndex);
                charVector.push_back(key);
            }
        }
        else
        {
            for(int index=0; index<lineStr.length();++index)
            {
                QChar key = lineStr.at(index);
                charVector.push_back(QString(key));

            }
        }


        m_resultVector.push_back(charVector);
    }
}

DoubleCharHandler::~DoubleCharHandler()
{

}

QVector<QVector<QString> > DoubleCharHandler::GetResult()
{
    return m_resultVector;
}

SpaceSplitHandler::SpaceSplitHandler(QString inputStr) : CharHandler(inputStr)
{
    QStringList lineList = m_targetStr.split("\r\n");
    for(int index=0; index<lineList.size(); ++index)
    {
        QString lineStr = lineList.at(index);
        QVector<QString> charVector;
        if(lineStr.contains(' '))
        {
            QStringList childStrList = lineStr.split(' ');
            int childStrSize = childStrList.size();
            for(int columnIndex=0; columnIndex<childStrSize; ++columnIndex)
            {
                QString key = childStrList.at(columnIndex);
                charVector.push_back(key);
            }
        }
        else
        {
            for(int index=0; index<lineStr.length();++index)
            {
                QChar key = lineStr.at(index);
                charVector.push_back(QString(key));

            }
        }


        m_resultVector.push_back(charVector);
    }
}

SpaceSplitHandler::~SpaceSplitHandler()
{

}

QVector<QVector<QString> > SpaceSplitHandler::GetResult()
{
    return m_resultVector;
}


