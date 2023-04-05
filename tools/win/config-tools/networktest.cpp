#include "networktest.h"
#include <QDebug>
#include <QDateTime>
#include "config.h"

NetWorkTest::NetWorkTest(QObject *parent) : QObject(parent)
{
    m_clientid = "00:99:22:33:44:55";
    m_loginid = "test13@thunisoft.com";
    m_password = "11163434";
    m_phoneNum = "1513434";
    m_versionNum = "7.0.0.0";
}

void NetWorkTest::RunTest()
{
    TestUserAccountLogin();
    TestGetVerificateionCode();

    TestClientVersionDetection();

    TestUploadUserConfigFile();
    TestDownloadUserConfigFile();

    TestUploadCustomWordlib();
    TestDownloadCustomWordlib();

    TestUploadCustomPhrase();
    TestDownloadCustomPhrase();

    TestUploadStatsInput();
    TestUploadFeedBack();

}


void NetWorkTest::TestUserAccountLogin()
{
    int returnCode = NetworkHandler::Instance()->UserAccountLogin(m_loginid.toLocal8Bit().toBase64(),m_password);
    if(returnCode == 0)
    {
        qDebug() << "账号登录接口测试通过";
    }
    else
    {
        qDebug() << QString("账号登录接口测试失败异常码:%1").arg(returnCode);
    }
}


void NetWorkTest::TestUserPhoneLogin()
{
}


void NetWorkTest::TestGetVerificateionCode()
{
    int returnCode = NetworkHandler::Instance()->GetVerificationCode(m_phoneNum);
    if(returnCode == 0)
    {
        qDebug() << "获取验证码接口测试通过";
    }
    else
    {
        qDebug() << QString("获取验证码接口测试失败异常码:%1").arg(returnCode);
    }
}


void NetWorkTest::TestClientVersionDetection()
{

}


void NetWorkTest::TestDownloadFileFromWeb()
{
}


void NetWorkTest::TestUploadUserConfigFile()
{
    qint64  time_value = QDateTime::currentDateTime().toMSecsSinceEpoch();
    QString current_sec_string = QString::number(time_value);
    QString config_file_path = Config::Instance()->configJsonFilePath();
    int returnCode = NetworkHandler::Instance()->UploadUserConfigFile(m_loginid,current_sec_string,config_file_path);
    if(returnCode == 0)
    {
        qDebug() << "用户配置文件上传接口测试通过";
    }
    else
    {
        qDebug() << QString("用户配置文件上传接口测试异常异常码:%1").arg(returnCode);
    }
}


void NetWorkTest::TestDownloadUserConfigFile()
{
     QString time_stamp;
     bool has_update;
     QString cfg_filePath;
     int returnCode = NetworkHandler::Instance()->DownloadUserConfigFile(m_loginid,time_stamp,has_update,cfg_filePath);
     if(returnCode == 0)
     {
         qDebug() << "下载用户配置文件接口测试通过";
     }
     else
     {
         qDebug() << QString("下载用户配置文件接口测试异常异常码:%1").arg(returnCode);
     }
}


void NetWorkTest::TestChangeUserPassword()
{
}


void NetWorkTest::TestUploadCustomWordlib()
{
    qint64  time_value = QDateTime::currentDateTime().toMSecsSinceEpoch();
    QString current_sec_string = QString::number(time_value);
    QString user_wordlib_file_path = Config::Instance()->UserWordlibFilePath();
    QString file_md5_value;
    int returnCode = NetworkHandler::Instance()->UploadCustomWordlib(m_loginid,m_clientid,user_wordlib_file_path,file_md5_value);
    if(returnCode == 0)
    {
        qDebug() << "上传用户词库接口测试通过";
    }
    else
    {
        qDebug() << QString("上传用户词库接口测试异常异常码:%1").arg(returnCode);
    }
}


void NetWorkTest::TestDownloadCustomWordlib()
{
    QString file_path;
    int returnCode = NetworkHandler::Instance()->DownloadCustomWordlib(m_loginid,m_clientid,file_path);
    if(returnCode == 0)
    {
        qDebug() << "下载用户词库测试通过";
    }
    else
    {
        qDebug() << QString("下载用户词库测试异常异常码:%1").arg(returnCode);
    }

}


void NetWorkTest::TestDownloadHighFreqWordlib()
{
}


void NetWorkTest::TestCheckSystemWordlib()
{
}


void NetWorkTest::TestCheckCommonWordlib()
{
}


void NetWorkTest::TestUploadCustomPhrase()
{
    qint64  time_value = QDateTime::currentDateTime().toMSecsSinceEpoch();
    QString current_sec_string = QString::number(time_value);

    QString custom_phrase_file_path = Config::Instance()->CustomPhraseFilePath();
    int returnCode = NetworkHandler::Instance()->UploadCustomPhraseFile(m_loginid,current_sec_string,custom_phrase_file_path);
    if(returnCode == 0)
    {
        qDebug() << "上传自定义短语接口测试通过";
    }
    else
    {
        qDebug() << QString("上传自定义短语接口测试异常异常码:%1").arg(returnCode);
    }
}


void NetWorkTest::TestDownloadCustomPhrase()
{
    QString time_stamp;
    QString download_url;
    bool has_update;
    int returnCode = NetworkHandler::Instance()->DownloadCutomPhraseFile(m_loginid,time_stamp,has_update,download_url);
    if(returnCode == 0)
    {
        qDebug() << "下载自定义短语接口测试通过";
    }
    else
    {
        qDebug() << QString("下载自定义短语接口测试异常异常码:%1").arg(returnCode);
    }
}


void NetWorkTest::TestUploadStatsInput()
{
    QString currentDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    int returnCode = NetworkHandler::Instance()->UploadInputStatsCount(m_clientid,m_loginid,m_versionNum,currentDateTime,20);
    if(returnCode == 0)
    {
        qDebug() << "上传统计量接口测试通过";
    }
    else
    {
        qDebug() << QString("上传统计量接口测试异常异常码:%1").arg(returnCode);
    }
}


void NetWorkTest::TestUploadFeedBack()
{
    QList<QString> imageList;
    int returnCode = NetworkHandler::Instance()->UploadFeedbackInfo(m_versionNum, m_clientid, m_loginid,"测试接口","测试反馈","1",imageList);
    if(returnCode == 0)
    {
        qDebug() << "上传反馈信息的接口测试通过";
    }
    else
    {
        qDebug() << QString("上传反馈信息测试异常异常码:%1").arg(returnCode);
    }
}

