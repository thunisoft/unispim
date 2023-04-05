#include "personaccountstackedwidget.h"
#include "ui_personaccountstackedwidget.h"
#include "config.h"
#include <QListView>
#include <QFile>
#include <QDateTime>
#include <QtConcurrent>
#include <fstream>

#include <QDir>
#include <QDialog>
#include "networkhandler.h"
#include "wordlibstackedwidget.h"
#include "utils.h"
#include "mainwindow.h"
#include "Windows.h"

PersonAccountStackedWidget::PersonAccountStackedWidget(QWidget *parent) :
    BaseStackedWidget(parent),
    ui(new Ui::PersonAccountStackedWidget)
{
    ui->setupUi(this);
    InitWidget();
    LoadConfigInfo();
    ConnectSignalToSlot();
}

PersonAccountStackedWidget::~PersonAccountStackedWidget()
{
    delete ui;
}

void PersonAccountStackedWidget::InitWidget()
{
    this->setProperty("type","personinfo");
    m_update_userwordlib_watcher = new QFutureWatcher<int>(this);
    m_update_userconfig_watcher = new QFutureWatcher<int>(this);
    m_update_phrase_watcher = new QFutureWatcher<int>(this);
    connect(m_update_userwordlib_watcher, &QFutureWatcher<int>::finished, this, &PersonAccountStackedWidget::slot_on_userwordlib_finished);
    connect(m_update_userconfig_watcher,&QFutureWatcher<int>::finished,this,&PersonAccountStackedWidget::slot_on_configfile_update_finished);
    connect(m_update_phrase_watcher,&QFutureWatcher<int>::finished,this,&PersonAccountStackedWidget::slot_on_phrasefile_update_finished);

    ui->person_center_btn->setProperty("type","normal-radius-set-btn");
    ui->logout_btn->setProperty("type","normal-radius-set-btn");
    ui->config_update_btn->setProperty("type","normal");
    ui->wordlib_update_btn->setProperty("type","normal");
    ui->account_option_label->setProperty("type","h1");
    ui->user_logo_label->setProperty("type","user_logo_label");
    ui->label->setProperty("type","h2");
    ui->label_3->setProperty("type","h2");
    this->setProperty("type","stackedwidget");
    QVector<int> labelLayout;
    bool isOk = Config::Instance()->GetLayoutVector("labellayout",labelLayout);
    if(isOk)
    {
        for(int index=0; index<labelLayout.size(); ++index)
        {
            ui->label_layout->setStretch(index,labelLayout.at(index));
        }
    }
    QStringList timeOptionList;
    timeOptionList << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8" << "9" << "10";
    ui->wordlib_update_time_combo->addItems(timeOptionList);
    ui->wordlib_update_time_combo->setView(new QListView);
    ui->wordlib_update_time_combo->setAutoFillBackground(true);
    ui->wordlib_update_time_combo->setMaxVisibleItems(6);
    ui->wordlib_update_time_combo->installEventFilter(this);
    ui->person_center_btn->setVisible(false);
    ui->logout_btn->setCursor(Qt::PointingHandCursor);
    ui->config_update_btn->setCursor(Qt::PointingHandCursor);
    ui->wordlib_update_btn->setCursor(Qt::PointingHandCursor);


}

void PersonAccountStackedWidget::ConnectSignalToSlot()
{
    connect(ui->person_center_btn, SIGNAL(clicked()), this, SLOT(SlotEnterPersonCenter()));
    connect(ui->logout_btn, SIGNAL(clicked()), this, SLOT(SlotLogout()));
    connect(ui->config_update_btn, SIGNAL(clicked()), this, SLOT(SlotAccountUpdate()));
    connect(ui->wordlib_update_btn, SIGNAL(clicked()), this, SLOT(SlotWordlibUpdate()));
}

void PersonAccountStackedWidget::LoadConfigInfo()
{
    ui->auto_update_account_checkbox->disconnect();
    ui->auto_update_wordlib_checkbox->disconnect();
    ui->wordlib_update_time_combo->disconnect();
    ConfigItemStruct configInfo;
    if(Config::Instance()->GetConfigItemByJson("account_auto_update",configInfo))
    {
       ui->auto_update_account_checkbox->setChecked(configInfo.itemCurrentIntValue);
    }
    if(Config::Instance()->GetConfigItemByJson("wordlib_auto_update",configInfo))
    {
        ui->auto_update_wordlib_checkbox->setChecked(configInfo.itemCurrentIntValue);
    }
    if(Config::Instance()->GetConfigItemByJson("loginid",configInfo))
    {
        ui->user_name_label->setText(configInfo.itemCurrentStrValue);
    }
    if(Config::Instance()->GetConfigItemByJson("auto_update_frequency",configInfo))
    {
        int index = ui->wordlib_update_time_combo->findText(QString::number(configInfo.itemCurrentIntValue));
        ui->wordlib_update_time_combo->setCurrentIndex(index);
    }else
    {
        int index = ui->wordlib_update_time_combo->findText("4");
        ui->wordlib_update_time_combo->setCurrentIndex(index);
    }
    connect(ui->auto_update_account_checkbox, SIGNAL(stateChanged(int)),this,SLOT(SlotEnableAccountUpdate(int)));
    connect(ui->auto_update_wordlib_checkbox, SIGNAL(stateChanged(int)),this,SLOT(SlotEnableWordlibUpdate(int)));
    connect(ui->wordlib_update_time_combo, SIGNAL(currentIndexChanged(int)),this, SLOT(SlotTimeComboIndexChanged(int)));

}

void PersonAccountStackedWidget::SlotEnterPersonCenter()
{

}

void PersonAccountStackedWidget::SlotLogout()
{
    //先保存当前用户的配置
    QString config_file_path = Config::Instance()->configJsonFilePath();
    QString loginid;
    ConfigItemStruct loginitem;
    if(Config::Instance()->GetConfigItemByJson("loginid",loginitem))
    {
        loginid = loginitem.itemCurrentStrValue;
    }
    QString phase_file_path = Config::Instance()->CustomPhraseFilePath();
    QString userUwlFilePath = Config::Instance()->UserWordlibFilePath();


    //更新用户配置文件
    QtConcurrent::run(this,&PersonAccountStackedWidget::SlotUpdateConfigFile,config_file_path,loginid);

    //更新用户自定义短语
    QtConcurrent::run(this,&PersonAccountStackedWidget::SlotUpdateCustomPhraseFile,phase_file_path,loginid);

    QtConcurrent::run(this,&PersonAccountStackedWidget::UpdateUserWordlib,userUwlFilePath,loginid);


    //修改用户登录状态
    ConfigItemStruct configInfo;
    configInfo.itemName = "login";
    configInfo.itemCurrentIntValue = 0;
    configInfo.itemGroupName = "state";
    Config::Instance()->SetStateConfigItemByJson("login",configInfo);
    Config::Instance()->SaveSystemConfig();

    Config::Instance()->LoadConfig();

    emit logout();

}

void PersonAccountStackedWidget::SlotAccountUpdate()
{
   //更新用户配置文件
   QString config_file_path = Config::Instance()->configJsonFilePath();
   QString loginid;
   ConfigItemStruct loginitem;
   if(Config::Instance()->GetConfigItemByJson("loginid",loginitem))
   {
       loginid = loginitem.itemCurrentStrValue;
   }
   QString phase_file_path = Config::Instance()->CustomPhraseFilePath();

   ui->config_update_btn->setEnabled(false);
   QFuture<int> res = QtConcurrent::run(this,&PersonAccountStackedWidget::SlotUpdateConfigFile,config_file_path,loginid);
   m_update_userconfig_watcher->setFuture(res);

   //更新用户自定义短语
   QFuture<int> phrase_res = QtConcurrent::run(this,&PersonAccountStackedWidget::SlotUpdateCustomPhraseFile,phase_file_path,loginid);
   m_update_phrase_watcher->setFuture(phrase_res);

}

void PersonAccountStackedWidget::SlotWordlibUpdate()
{
    SlotUpdateUserWordlibFile();
}

int PersonAccountStackedWidget::RestoreUserWordlib()
{
    try
    {
        QString wordlib_path = Config::Instance()->UserWordlibFilePath();
        QString bak_wordlib_path = wordlib_path + ".bak.tools";
        if(!QFile::exists(wordlib_path))
        {
            return 1;
        }

        QFile uwl_file(wordlib_path);
        QFile bak_wordlib_file(bak_wordlib_path);

        auto restore_bak_file = [&](){
            if(bak_wordlib_file.exists())
            {
                if(IsWordlibValid(bak_wordlib_path))
                {
                    Utils::WriteLogToFile("user.uwl is error and user.uwl.bak is ok ");
                    uwl_file.remove();
                    bak_wordlib_file.rename(wordlib_path);
                }
                else
                {
                    Utils::WriteLogToFile("user.uwl is error and user.uwl.bak is error ");
                    uwl_file.remove();
                    bak_wordlib_file.remove();
                }

            }
            else
            {
                Utils::WriteLogToFile("bak file doesn't exist");
                uwl_file.remove();
            }

        };


        int file_size = uwl_file.size();
        if((file_size % 1024) != 0)
        {
            Utils::WriteLogToFile("when restore user.uwl size is not valid");
            restore_bak_file();
            return 0;
        }
        if(!IsWordlibValid(wordlib_path))
        {
            restore_bak_file();
        }
        return 0;

    }
    catch(std::exception& e)
    {
        e;
        return 1;
    }
}

bool PersonAccountStackedWidget::IsWordlibValid(const QString &wordlib_path)
{
    try
    {
        if(!QFile::exists(wordlib_path))
        {
            return false;
        }

        QFile uwl_file(wordlib_path);
        int file_size = uwl_file.size();
        Utils::WriteLogToFile(QString("%1 filesize is: %2").arg(wordlib_path).arg(file_size));
        if((file_size % 1024) != 0)
        {
            Utils::WriteLogToFile("filesize is unvalid");
            return false;
        }


        QProcess* wordlib_process = new QProcess(this);
        QFileInfo wordlib_file_info(wordlib_path);

        QString file_dir_path = wordlib_file_info.absolutePath();
        QString txt_file_name = wordlib_file_info.baseName() + ".txt";
        QString target_txt_path = QDir::toNativeSeparators( file_dir_path + "/" + txt_file_name);

        QStringList params;
        params << QString("-export");
        params << QString(wordlib_path);
        params << QString(target_txt_path);

        QString wordlib_tool_path = Config::Instance()->GetInstallDir().append("\\UwlValidator.exe");
        //one minute
        wordlib_process->execute(wordlib_tool_path, params);


        QFile file(target_txt_path);
        if (!file.exists())
        {
            Utils::WriteLogToFile("txt file is not exist");
            return false;
        }

        int current_file_count = 0;
        QFile txt_file(target_txt_path);
        if (txt_file.open(QIODevice::ReadOnly))
        {
            while (!txt_file.atEnd())
            {
                QByteArray tmp = txt_file.readLine();
                {
                    current_file_count++;
                }
            }
            txt_file.close();
        }
        Utils::WriteLogToFile(QString("txt wordlib file count is: %2").arg(current_file_count));

        if(current_file_count >= 5)
        {
            QFile::remove(target_txt_path);
            return true;
        }
        else
        {
            QFile::remove(target_txt_path);
            return false;
        }

    }
    catch(std::exception& e)
    {
        e;
        Utils::WriteLogToFile("when valid throw exception");
        return false;
    }
}

void PersonAccountStackedWidget::SlotEnableAccountUpdate(int flag)
{
    ConfigItemStruct configInfo;
    if(flag == Qt::Checked)
    {
        configInfo.itemName = "account_auto_update";
        configInfo.itemCurrentIntValue = 1;
        Config::Instance()->SetConfigItemByJson("account_auto_update",configInfo);
    }
    else
    {
        configInfo.itemName = "account_auto_update";
        configInfo.itemCurrentIntValue = 0;
        Config::Instance()->SetConfigItemByJson("account_auto_update",configInfo);
    }
    Config::Instance()->SaveConfig();
}

void PersonAccountStackedWidget::SlotEnableWordlibUpdate(int flag)
{
    ConfigItemStruct configInfo;
    if(flag == Qt::Checked)
    {
        configInfo.itemName = "wordlib_auto_update";
        configInfo.itemCurrentIntValue = 1;
        Config::Instance()->SetConfigItemByJson("wordlib_auto_update",configInfo);
    }
    else
    {
        configInfo.itemName = "wordlib_auto_update";
        configInfo.itemCurrentIntValue = 0;
        Config::Instance()->SetConfigItemByJson("wordlib_auto_update",configInfo);
    }
    Config::Instance()->SaveConfig();
}

void PersonAccountStackedWidget::SlotTimeComboIndexChanged(int index)
{
    ConfigItemStruct configInfo;
    int timeFrequency = ui->wordlib_update_time_combo->itemText(index).toInt();

    configInfo.itemName = "auto_update_frequency";
    configInfo.itemCurrentIntValue = timeFrequency;
    Config::Instance()->SetConfigItemByJson("auto_update_frequency",configInfo);
    Config::Instance()->SaveConfig();
}

void PersonAccountStackedWidget::SetUserState(bool isLogin)
{
    if(isLogin)
    {
        LoadConfigInfo();
    }
}

int PersonAccountStackedWidget::SlotUpdateCustomPhraseFile(QString phrase_file_path, QString loginid)
{
    NetworkHandler network_handler;
    QString customPhraseFilePath = phrase_file_path;
    QString serverTimeStamp;
    QString customPhraseFileAddr;

    bool hasUpdate = false;

    QFileInfo fileInfo(customPhraseFilePath);
    QString customPhraseFileDir = fileInfo.dir().path();
    QString targetFilePath;


    //自定义短语
    if(QFile::exists(customPhraseFilePath))
    {

        qint64 currentTimeStamp = fileInfo.lastModified().toMSecsSinceEpoch();

        int returnCode = network_handler.DownloadCutomPhraseFile(loginid,serverTimeStamp,hasUpdate,customPhraseFileAddr);
        if( returnCode == 0)
        {
            qint64 serverValue= serverTimeStamp.toLongLong();
            //时间戳相同则不做操作
            if(serverValue == currentTimeStamp)
            {
                return 0;
            }
            //时间戳小于服务器，对用户自定义短语进行更新
            else if(currentTimeStamp < serverValue)
            {

                int returnCode = network_handler.DownloadFileFromWeb(customPhraseFileAddr,customPhraseFileDir,targetFilePath,"user_phrase.txt");
                if(returnCode == 0)
                {
                    //Utils::CopyPhraseFileToTmpDir(loginid);
                }
                return returnCode;

            }
            //时间戳大于服务器则上传
            else if(currentTimeStamp > serverValue)
            {
                int uploadeValue = network_handler.UploadCustomPhraseFile(loginid,QString::number(currentTimeStamp),customPhraseFilePath);
                return uploadeValue;
            }

        }
        //服务端不存在则上传
        else if(returnCode == 311)
        {
            int uploadValue = network_handler.UploadCustomPhraseFile(loginid,QString::number(currentTimeStamp),customPhraseFilePath);
            return uploadValue;
        }
        else
        {
            return returnCode;
        }
    }
    //配置文件不存在的情况
    else
    {
        int returnCode = network_handler.DownloadCutomPhraseFile(loginid,serverTimeStamp,hasUpdate,customPhraseFileAddr);
        if(returnCode == 0)
        {
            {
                int downloadCode = network_handler.DownloadFileFromWeb(customPhraseFileAddr,customPhraseFileDir,targetFilePath,"user_phrase.txt");
                if(downloadCode == 0)
                {
                    //Utils::CopyPhraseFileToTmpDir(loginid);
                }
                return downloadCode;
            }

        }
        //本地没有配置文件服务端也没有则不提示
        else if(returnCode == 311)
        {
            return 0;
        }
        return returnCode;
    }
    return 1;
}

int PersonAccountStackedWidget::LoginedUpdateUserWordlib()
{
    QString clientid = Config::Instance()->GetHostMacAddress();
    QString loginid = Config::Instance()->GetLoginID();

    QString  server_md5, prev_clientid,server_url_addr;
    NetworkHandler request_handler;
    int getmd5_ret = request_handler.GetCurrentUserUserwordlibMD5(loginid, clientid, server_md5, server_url_addr, prev_clientid);
    if (getmd5_ret != 0)
    {
        Utils::WriteLogToFile(QString("logined state get last upload clientid error:%1").arg(getmd5_ret));
        return 1;
    }
    if (prev_clientid == clientid)
    {
        Utils::WriteLogToFile("last time upload at the same pc");
    }
    else
    {
        Utils::WriteLogToFile("last time upload in a different pc");
        int ret_code = DownloadAndCombineUserWordlib();
        if (ret_code != 0)
        {
            Utils::WriteLogToFile(QString("logined state download and combine wordlib error:%1").arg(ret_code));
            return 2;
        }
    }

    QString userUwlFilePath = Utils::GetUserWordlibDirPath() + "user.uwl";
    QString upload_result_md5;
    int upload_ret = request_handler.UploadCustomWordlib(loginid, clientid, userUwlFilePath, upload_result_md5);
    if (upload_ret == 0)
    {

        Utils::WriteLogToFile("logined state upload wordlib succeed");
        return 0;
    }
    else if (upload_ret == 710)
    {
        Utils::WriteLogToFile("logined state upload wordlib return 710");
        int ret = -1;
        if (prev_clientid == clientid)
        {
            ret = DownloadAndCombineUserWordlib();
        }
        else
        {
            ret = DownloadAndReplaceLocalUserWordlib();
            Utils::WriteLogToFile(QString("download and replace wordlib return:%1").arg(ret));
        }
        if(ret == 0)
        {
            return 0;
        }
    }
    else
    {
        Utils::WriteLogToFile(QString("unlogined state upload user worlid error-ret:%1").arg(upload_ret));
    }
    return upload_ret;
}

int PersonAccountStackedWidget::DownloadAndCombineUserWordlib()
{
    try
    {
        NetworkHandler network_handler;
        QString loginid = Config::Instance()->GetLoginID();
        QString clientid = Config::Instance()->GetHostMacAddress();
        QString userUwlFilePath = Utils::GetUserWordlibDirPath() + "user.uwl";
        QString server_user_wordlib_md5, server_user_wordlib_url,prev_clientid;
        QString userUwlFilePathTmp = userUwlFilePath + QString(".tmp.tools");
        QString userUwlFIlePathBak = userUwlFilePath + QString(".bak.tools");
        QString local_for_combine = userUwlFilePath + QString(".tmp.combine");
        QString local_for_combine_bak = userUwlFilePath + QString(".tmp.combine.bak");

        int getmd5_ret = network_handler.GetCurrentUserUserwordlibMD5(loginid, clientid, server_user_wordlib_md5, server_user_wordlib_url, prev_clientid);
        if ((getmd5_ret == 0) && !server_user_wordlib_md5.isEmpty() && !server_user_wordlib_url.isEmpty())
        {
            Utils::WriteLogToFile("when download get server info succeed");
            //如果服务器端和本地端的词库MD5值不同下载词库
            QFile userUwlFileTmp(userUwlFilePathTmp);
            if (userUwlFileTmp.exists())
                userUwlFileTmp.remove();

            QFile userUwlFileBak(userUwlFIlePathBak);
            if (userUwlFileBak.exists())
                userUwlFileBak.remove();

            QFile userwordlib(userUwlFilePath);

            int downloadReturnCode = network_handler.DownloadFileFromWeb(server_user_wordlib_url,userUwlFilePathTmp);
            if (downloadReturnCode != 0)
            {
                Utils::WriteLogToFile(QString("download useruwl failed return:%1").arg(downloadReturnCode));
                return 1;
            }
            QString download_md5 = Utils::GetFileMD5(userUwlFilePathTmp);

            if ((IsWordlibValid(userUwlFilePathTmp)) && (download_md5 == server_user_wordlib_md5))
            {
                Utils::WriteLogToFile("download user.uwl succeed");
                if (!userwordlib.exists())
                {
                    userUwlFileTmp.rename(userUwlFilePath);
                    return 0;
                }
                else
                {
                    userwordlib.copy(local_for_combine);
                    bool combine_ret = Utils::combine_wordlib(local_for_combine, userUwlFilePathTmp);

                    QFile local_for_combine_bak_file(local_for_combine_bak);
                    QFile local_combine_file(local_for_combine);
                    if ((IsWordlibValid(local_for_combine)) && combine_ret)
                    {
                        Utils::WriteLogToFile("combine server user.uwl and local user.uwl succeed");
                        userwordlib.rename(userUwlFIlePathBak);
                        local_combine_file.rename(userUwlFilePath);
                        userUwlFileTmp.remove();
                        if (local_for_combine_bak_file.exists())
                        {
                            local_for_combine_bak_file.remove();
                        }
                        return 0;
                    }
                    local_combine_file.remove();
                    userUwlFileTmp.remove();
                    if (local_for_combine_bak_file.exists())
                    {
                        local_for_combine_bak_file.remove();
                    }
                    return 2;

                }
            }
            else
            {
                Utils::WriteLogToFile("download useruwl file is not valid");
                userUwlFileTmp.remove();
                return 3;
            }
        }
        else
        {
            Utils::WriteLogToFile(QString("get server info error code:%1,md5:2,url:%3").arg(getmd5_ret).arg(server_user_wordlib_md5).arg(server_user_wordlib_url));
            return 4;
        }
    }
    catch (std::exception& e)
    {
        Utils::WriteLogToFile(QString("download and combine wordlib throw exception:%1").arg(e.what()));
        return 5;
    }
}

int PersonAccountStackedWidget::DownloadAndReplaceLocalUserWordlib()
{
    try
        {
            Utils::WriteLogToFile("runinto download and replace process");
            QString loginid = Config::Instance()->GetLoginID();
            QString clientid = Config::Instance()->GetHostMacAddress();
            QString userUwlFilePath = Utils::GetUserWordlibDirPath() + "user.uwl";
            QString server_user_wordlib_md5, server_user_wordlib_url,prev_clientid;
            QString userUwlFilePathTmp = userUwlFilePath + QString(".tmp.tools");
            QString userUwlFIlePathBak = userUwlFilePath + QString(".bak.tools");
            NetworkHandler network_handler;

            int getmd5_ret = network_handler.GetCurrentUserUserwordlibMD5(loginid, clientid, server_user_wordlib_md5, server_user_wordlib_url, prev_clientid);
            if ((getmd5_ret == 0) && !server_user_wordlib_md5.isEmpty() && !server_user_wordlib_url.isEmpty())
            {
                Utils::WriteLogToFile("when download get server info succeed");
                //如果服务器端和本地端的词库MD5值不同下载词库
                QFile userUwlFileTmp(userUwlFilePathTmp);
                if (userUwlFileTmp.exists())
                    userUwlFileTmp.remove();

                QFile userUwlFileBak(userUwlFIlePathBak);
                if (userUwlFileBak.exists())
                    userUwlFileBak.remove();

                QFile userwordlib(userUwlFilePath);

                int downloadReturnCode = network_handler.DownloadFileFromWeb(server_user_wordlib_url, userUwlFilePathTmp);
                if (downloadReturnCode != 0)
                {
                    Utils::WriteLogToFile(QString("download  useruwl failed return:%1").arg(downloadReturnCode));
                    return 1;
                }
                QString download_md5 = Utils::GetFileMD5(userUwlFilePathTmp);

                if ((IsWordlibValid(userUwlFilePathTmp)) && (download_md5 == server_user_wordlib_md5))
                {
                    Utils::WriteLogToFile("download user.uwl succeed");
                    if (userwordlib.exists())
                    {
                        userwordlib.rename(userUwlFIlePathBak);
                        userUwlFileTmp.rename(userUwlFilePath);
                        return 0;
                    }
                    else
                    {
                        userUwlFileTmp.rename(userUwlFilePath);
                        return 0;
                    }
                }
                else
                {
                    Utils::WriteLogToFile("download useruwl file is not valid");
                    userUwlFileTmp.remove();
                    return 3;
                }
            }
            else
            {
                return 4;
            }
        }
        catch (std::exception& e)
        {
            Utils::WriteLogToFile(QString("download and replace wordlib throw exception:%1").arg(e.what()));
            return 5;
        }
}

int PersonAccountStackedWidget::SlotUpdateConfigFile(QString config_file_path, QString loginid)
{
    NetworkHandler networker;
    QString configJsonFilePath = config_file_path;
    QString serverTimeStamp;
    QString cfgFileAddr;
    bool hasUpdate = false;


    QFileInfo configFileInfo(configJsonFilePath);
    QString configFileDir = configFileInfo.dir().path();
    QString targetFilePath;

    //配置文件存在的情况
    if(QFile::exists(configJsonFilePath))
    {
        qint64 currentTimeStamp = configFileInfo.lastModified().toMSecsSinceEpoch();

        int returnCode = networker.DownloadUserConfigFile(loginid,serverTimeStamp,hasUpdate,cfgFileAddr);
        if( returnCode == 0)
        {
            qint64 serverValue= serverTimeStamp.toLongLong();
            Utils::WriteLogToFile("Server has config file update");
            //时间戳相同则不做操作
            if(serverValue == currentTimeStamp)
            {
                 Utils::WriteLogToFile("User config file already uptodate");
                 return 0;
            }
            //时间戳小于服务器，则对配置文件进行更新
            else if(currentTimeStamp < serverValue)
            {

                int returnCode = networker.DownloadFileFromWeb(cfgFileAddr,configFileDir,targetFilePath,"config.json");
                if(returnCode == 0)
                {

                    Utils::WriteLogToFile("Download user config file succeed");
                    return 0;
                }
                else
                {
                    Utils::WriteLogToFile(QString("download user config file Error:%1").arg(returnCode));
                    return 1;
                }

            }
            //时间戳大于服务器则上传
            else if(currentTimeStamp > serverValue)
            {
                int uploadeValue = networker.UploadUserConfigFile(loginid,QString::number(currentTimeStamp),configJsonFilePath);
                if(uploadeValue == 0)
                {
                    Utils::WriteLogToFile("User config file upload succeed");
                    return 0;
                }
                return uploadeValue;
            }

        }
        //服务端不存在则上传
        else if(returnCode == 311)
        {

            Utils::WriteLogToFile("Server don't have user config File");
            int uploadValue = networker.UploadUserConfigFile(loginid,QString::number(currentTimeStamp),configJsonFilePath);
            if(uploadValue == 0)
            {
                Utils::WriteLogToFile("User Config File update succeed");
                return 0;
            }
        }
        else
        {
            Utils::WriteLogToFile(QString("downlaod user config file error:%1").arg(returnCode));
            return 1;
        }
    }
    //配置文件不存在的情况
    else
    {
        Utils::WriteLogToFile("Local User config file don't exist ");
        int returnCode = networker.DownloadUserConfigFile(loginid,serverTimeStamp,hasUpdate,cfgFileAddr);
        if( returnCode == 0)
        {
            if(!hasUpdate)
            {
                return 0;
            }

            int download_code = networker.DownloadFileFromWeb(cfgFileAddr,configFileDir,targetFilePath,"config.json");
            if(download_code == 0)
            {
               return 0;
            }
            return download_code;


        }
        //本地没有配置文件服务端也没有则不提示
        else if(returnCode == 311)
        {
            return 0;
        }
        else
        {
            return 1;

        }
    }
    return 1;
}

int PersonAccountStackedWidget::SlotUpdateUserWordlibFile()
{
    QString userUwlFilePath = Config::Instance()->UserWordlibFilePath();
    ConfigItemStruct configInfo;
    QString loginid;
    if(Config::Instance()->GetConfigItemByJson("loginid", configInfo))
    {
        loginid = configInfo.itemCurrentStrValue;
    }

    RestoreUserWordlib();

    QFuture<int> update_wordlib_future = QtConcurrent::run(this,&PersonAccountStackedWidget::UpdateUserWordlib,userUwlFilePath,loginid);
    m_update_userwordlib_watcher->setFuture(update_wordlib_future);
    ui->wordlib_update_btn->setEnabled(false);
    return 0;
}

void PersonAccountStackedWidget::slot_on_userwordlib_finished()
{
    ui->wordlib_update_btn->setEnabled(true);
    QFuture<int> result = m_update_userwordlib_watcher->future();
    int returnCode = result.resultAt(0);
    if(returnCode == 701)
    {
        Utils::WriteLogToFile("Server user wordlib is Empty");
        Utils::NoticeMsgBox("服务端用户词库为空",this);

    }
    else if(returnCode == 0)
    {
        Config::Instance()->SaveConfig();
        emit configChanged();
        emit userInfoHasUpdated();
        Utils::WriteLogToFile("User wordlib update succeed");
        Utils::NoticeMsgBox("用户词库同步成功",this);
    }
    else if(returnCode == 666)
    {
        Utils::WriteLogToFile(QString("User Wordlib update time out"));
        Utils::NoticeMsgBox("请求超时",this);
    }
    else
    {
        Utils::WriteLogToFile(QString("User Wordlib update error:%1").arg(returnCode));
        Utils::NoticeMsgBox("用户词库同步异常",this);
    }
}

void PersonAccountStackedWidget::slot_on_configfile_update_finished()
{
    int returnCode = m_update_userconfig_watcher->future().result();
    if(m_update_phrase_watcher->isFinished() && m_update_userconfig_watcher->isFinished())
    {
        ui->config_update_btn->setEnabled(true);
    }
    switch(returnCode){
    case 0:
        emit userconfigfileupdated();
        Utils::NoticeMsgBox("配置文件同步成功",this);
        break;
    case 666:
        Utils::NoticeMsgBox("配置文件同步超时",this);
        break;
    default:
        Utils::NoticeMsgBox("配置文件同步失败",this);
        break;
    }
}

void PersonAccountStackedWidget::slot_on_phrasefile_update_finished()
{
    int returnCode = m_update_phrase_watcher->future().result();
    if(m_update_phrase_watcher->isFinished() && m_update_userconfig_watcher->isFinished())
    {
        ui->config_update_btn->setEnabled(true);
    }

    switch(returnCode){
    case 0:
        emit userInfoHasUpdated();
        Utils::NoticeMsgBox("自定义短语文件同步成功",this);
        break;
    case 666:
        Utils::NoticeMsgBox("自定义短语文件同步超时",this);
        break;
    default:
        Utils::NoticeMsgBox("自定义短语文件同步失败",this);
        break;
    }
}

int PersonAccountStackedWidget::UpdateUserWordlib(QString user_wordlib_path,QString loginid)
{
    QString userUwlFilePath = user_wordlib_path;
    QString clientid = Config::Instance()->GetHostMacAddress();
    NetworkHandler request_handler;
    QString serverMD5Value;

    //用户词库存在的时候
    if(QFile::exists(userUwlFilePath))
    {
        Utils::WriteLogToFile("Local User wordlib exists");
        QString server_user_wordlib_md5, server_user_wordlib_url,perv_clientid;
        int getmd5_ret = request_handler.GetCurrentUserUserwordlibMD5(loginid,clientid,server_user_wordlib_md5,server_user_wordlib_url,perv_clientid);
        if(getmd5_ret != 0)
        {
            Utils::WriteLogToFile(QString("get server user wordlib md5 error:%1").arg(getmd5_ret));
            return 1;
        }
        if(server_user_wordlib_md5.isEmpty())
        {
            Utils::WriteLogToFile("server user wordlib md5 is empty");
            return 2;
        }

        QString local_user_md5 = Utils::GetFileMD5(userUwlFilePath);
        if(server_user_wordlib_md5 == local_user_md5)
        {
            Utils::WriteLogToFile("user.wordlib server md5 is same with local file md5");
            return 0;
        }
        if (loginid.isEmpty())
        {
            Utils::WriteLogToFile( "unlogined state update user wordlib");
            int returnCode = request_handler.UploadCustomWordlib(loginid,clientid,userUwlFilePath,serverMD5Value);
            if(returnCode == 0)
            {
                Utils::WriteLogToFile("unlogined state upload user wordlib succeed");
                return 0;
            }
            else if(returnCode == 710)
            {
                int ret = DownloadAndCombineUserWordlib();
                Utils::WriteLogToFile(QString("unlogined state upload wordlib return 710 download and combine:%1").arg(ret));
                return 1;
            }
            else
            {
                Utils::WriteLogToFile(QString("unlogined state upload user wordlib error:%1").arg(returnCode));
                return 2;
            }
        }
        else
        {
            int ret = LoginedUpdateUserWordlib();
            Utils::WriteLogToFile(QString("logined state process return:%1").arg(ret));
            return 0;
        }
    }
    else
    {
       int ret = DownloadAndCombineUserWordlib();
       if(ret != 0)
       {
           Utils::WriteLogToFile(QString("when local user.uwl doesn't exist downloadandcombine return:%1").arg(ret));
           return ret;
       }
       return 0;
    }
    return 1;
}

int PersonAccountStackedWidget::asyn_update_userwordlib()
{
    QString userUwlFilePath = Config::Instance()->UserWordlibFilePath();
    ConfigItemStruct configInfo;
    QString loginid;
    if(Config::Instance()->GetConfigItemByJson("loginid", configInfo))
    {
        loginid = configInfo.itemCurrentStrValue;
    }


    int ret = UpdateUserWordlib(userUwlFilePath,loginid);
    if(ret == 0)
    {
        MainWindow::SendMsgToEngine();
        return 0;
    }
    return 1;
}

bool PersonAccountStackedWidget::eventFilter(QObject *obj, QEvent *event)
{
    if(obj->metaObject()->className() == QStringLiteral("QComboBox"))
    {
        if (event->type() == QEvent::Wheel)//鼠标滚轮事件
        {
            return true;//禁用下拉框的滚轮改变项的功能
        }
    }
    return false;
}
