#include "uploadcache.h"
#include "../public/config.h"

#include <QDir>
#include <QSettings>

UploadCache* UploadCache::gInstance = NULL;


UploadCache* UploadCache::GetInstance()
{
    if(gInstance == NULL)
    {
        gInstance = new UploadCache;
    }
    return gInstance;
}

UploadCache::UploadCache(QObject *parent) : QObject(parent)
{

}

UploadCache::~UploadCache()
{
}

int UploadCache::GetCacheSize()
{
    return m_cacheVector.size();
}

QVector<QPair<QString,int> > UploadCache::GetCacheInfo()
{
    return m_cacheVector;
}

void UploadCache::PushIntoCache(const QPair<QString, int> inputPair)
{
    OutputToFile(inputPair.first, inputPair.second);
}

void UploadCache::ClearCacheInfo()
{
    m_cacheVector.clear();
}

void UploadCache::LoadCacheData()
{
    QString cacheFilePath = GetSavePath();
    bool isFileExist = QFile::exists(cacheFilePath);
    if(!isFileExist)
    {
        return;
    }
    m_cacheVector.clear();
    QSettings  *setting = new QSettings(cacheFilePath,QSettings::IniFormat);
    setting->beginGroup("CacheInfo");
    QStringList childKeys = setting->childKeys();
    int keyNum = childKeys.size();
    for(int index=0; index<keyNum; ++index)
    {
        QString currentKey = childKeys.at(index);
        int value = setting->value(currentKey).toInt();
        QPair<QString, int> tempPair = qMakePair(currentKey, value);
        m_cacheVector.push_back(tempPair);
    }
    setting->endGroup();
    delete setting;
}

void UploadCache::OutputToFile(const QString key, const int value)
{

    QString cacheFilePath = GetSavePath();
    QSettings *setting = new QSettings(cacheFilePath, QSettings::IniFormat);
    setting->beginGroup("CacheInfo");
    setting->setValue(key,value);
    setting->endGroup();
    delete setting;
    setting = NULL;
}

void UploadCache::removeItem(const QString key)
{
    QString savePath = GetSavePath();
    QSettings *setting = new QSettings(savePath,QSettings::IniFormat);
    setting->beginGroup("CacheInfo");

    QVector<QPair<QString,int> >::iterator itor = m_cacheVector.begin();
    while(itor != m_cacheVector.end())
    {
        QPair<QString, int> currentPair = *itor;
        if(key == currentPair.first)
        {
            itor = m_cacheVector.erase(itor);
            QStringList keyLists = setting->allKeys();
            if(keyLists.contains(key))
            {
                setting->remove(key);
            }
        }
        else
        {
             ++itor;
        }

    }
    setting->endGroup();
    delete setting;
    setting = NULL;
}

QString UploadCache::GetSavePath()
{
    QString loginID;
    if(!Config::GetClientInfo(loginID))
    {
        loginID = "default";
    }
    QString currentPath = QDir::homePath();
    QString cacheFilePath = QDir::toNativeSeparators(currentPath + QString("/.config/huayupy/uploadCache-%1.txt").arg(loginID));
    return cacheFilePath;
}






















