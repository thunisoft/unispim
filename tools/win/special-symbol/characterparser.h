#ifndef CHARACTERPARSER_H
#define CHARACTERPARSER_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QPair>
#include <QDebug>

struct SYMBOL_INFO{

    QString symbolName;
    QString symbolDesctribe;
};

class CharacterParser : public QObject
{
    Q_OBJECT

public:
    static CharacterParser* Instance();
private:
    explicit CharacterParser(QObject *parent = nullptr);
public:
    ~CharacterParser();
public:
    bool ParserFile();
    void SetSymbolFileAddr(const QString &fileAddr);
    QVector<QPair<QString,QString> > GetResult();

    bool OuputSymbolToCSV(QString filePath);

    QString GetSymbolDescribe(QString titlename, QString symbol);

    QVector<QString> GetSearchResult(QString searchIndex);

    bool ParserSymbolDescribeFile(QString sourcePath);

Q_SIGNALS:

private:
    QVector<QPair<QString, QString> > m_symbol_vector;
    QString m_symbolFile_addr;

    QMap<QString, QVector<SYMBOL_INFO> > m_symbolDescribeMap;

    static CharacterParser* m_instance;
};

#endif // CHARACTERPARSER_H
