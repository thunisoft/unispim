#ifndef UPLOADCACHE_H
#define UPLOADCACHE_H

#include <QObject>
#include <QVector>
#include <QPair>

class UploadCache : public QObject
{
    Q_OBJECT
public:
    static UploadCache* GetInstance();
private:
    explicit UploadCache(QObject *parent = 0);
public:
    ~UploadCache();

public:
    void PushIntoCache(const QPair<QString,int> inputPair);
    QVector<QPair<QString, int> > GetCacheInfo();
    void ClearCacheInfo();
    int GetCacheSize();
    void removeItem(const QString key);
    void LoadCacheData();
private:
    QString GetSavePath();
    void OutputToFile(const QString key, const int value);

private:
    static UploadCache* gInstance;
    QVector<QPair<QString, int> > m_cacheVector;
};

#endif // UPLOADCACHE_H
