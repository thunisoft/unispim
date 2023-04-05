#ifndef CHARHANDLER_H
#define CHARHANDLER_H

#include <QVector>


class CharHandler
{
public:
    CharHandler(QString inputStr);
public:
    virtual ~CharHandler();

public:
      virtual QVector<QVector<QString> > GetResult() = 0;

protected:
    QVector<QVector<QString> > m_resultVector;
    QString m_targetStr;

};

class SingleCharHandler : public CharHandler
{
public:
    SingleCharHandler(QString inputStr);
    ~SingleCharHandler();
public:
    QVector<QVector<QString> > GetResult();
};

class DoubleCharHandler : public CharHandler
{
 public:
    DoubleCharHandler(QString inputStr);
    ~DoubleCharHandler();
 public:
    QVector<QVector<QString> > GetResult();
};


class SpaceSplitHandler : public CharHandler
{
 public:
    SpaceSplitHandler(QString inputStr);
    ~SpaceSplitHandler();
 public:
    QVector<QVector<QString> > GetResult();
};

#endif // CHARHANDLER_H
