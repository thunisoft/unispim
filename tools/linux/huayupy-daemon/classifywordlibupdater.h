//#ifndef CLASSIFYWORDLIBUPDATER_H
//#define CLASSIFYWORDLIBUPDATER_H

//#include <QList>
//#include <QObject>
//#include <QMap>
//#include "../public/defines.h"

//class QNetworkAccessManager;
//class QNetworkReply;
//class QJsonValueRef;



//enum Status
//{
//   UNWORK = 0,
//   WORKING
//};

//class ClassifyWordlibUpdater : public QObject
//{
//    Q_OBJECT
//public:
//    explicit ClassifyWordlibUpdater(QObject *parent = 0);
//    void Update();

//private slots:
//    void OnOneWordlibDownloadFinished(QNetworkReply*);
//    void OnGetWordlibInfoDone(QNetworkReply* reply);

//private:
//    void Init();
//    void GetLocalWordlib();
//    void GetServerWordlib();
//    void GetWordlibToUpdate(QList<WORDLIB_INFO>& wordlib_info_list);
//    void SetStatusWorking();
//    void SetStatusUnWork();
//    Status GetStatus();
//    void PhraseJsonInfo(const QByteArray& data);
//    void FillDataToWordlibInfoEntity(WORDLIB_INFO& wordlib_info, QJsonValueRef json_value);
//    void DownloadWordlib();

//    Status m_status;
//    QNetworkAccessManager *m_network_manager;
//    int m_download_finished_count;
//    QList<WORDLIB_INFO> m_wordlibs_to_check;
//    QMap<QString, WORDLIB_INFO> m_local_wordlibs_info;
//    QMap<QString, WORDLIB_INFO> m_server_wordlibs_info;


//};

//#endif // CLASSIFYWORDLIBUPDATER_H
