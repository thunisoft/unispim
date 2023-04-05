#ifndef CHARACTERPARSER_H
#define CHARACTERPARSER_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QPair>
#include <QDebug>
class CharacterParser : public QObject
{
    Q_OBJECT
public:
    explicit CharacterParser(QObject *parent = 0);
    bool ParserFile();
    void SetSymbolFileAddr(const QString &fileAddr);
    QVector<QPair<QString,QString> > GetResult();

Q_SIGNALS:

private:
    QVector<QPair<QString, QString> > m_symbol_vector;
    QString m_symbolFile_addr;
};

#endif // CHARACTERPARSER_H
