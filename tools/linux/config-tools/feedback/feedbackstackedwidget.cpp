#include "feedbackstackedwidget.h"
#include "ui_feedbackstackedwidget.h"
#include "../public/configmanager.h"
#include "../wordlibpage/myutils.h"
#include "../public/utils.h"
#include <QMimeData>
#include <QUrl>
#include <QList>
#include <QTextBlock>
#include <QFileDialog>
#include <QDesktopServices>

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QSslSocket>
#include <QHttpMultiPart>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QSharedPointer>
#include <customize_ui/customizefont.h>
#include "msgboxutils.h"

FeedbackStackedWidget::FeedbackStackedWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FeedbackStackedWidget)
{
    ui->setupUi(this);
    InitWidget();
    ConnectSignalToSlot();
}

FeedbackStackedWidget::~FeedbackStackedWidget()
{
    delete ui;
}

void FeedbackStackedWidget::ResetConfigInfo()
{
    ui->bug_radio_btn->setChecked(true);
    ui->textEdit->clear();
}

void FeedbackStackedWidget::InitWidget()
{
    m_typeValue = 1;

    QFile file(":/feedback/basicstyle.qss");
    if(file.open(QIODevice::ReadOnly))
    {
        this->setStyleSheet(file.readAll());
        file.close();
    }


    QList<QRadioButton*> childRaidoList = this->findChildren<QRadioButton*>();
    for(QRadioButton* index : childRaidoList)
    {
       index->setFocusPolicy(Qt::NoFocus);
    }

    QList<QPushButton*> childButtonList = this->findChildren<QPushButton*>();
    for(QPushButton* index : childButtonList)
    {
       index->setFocusPolicy(Qt::NoFocus);
    }
    QFont h1Font = CustomizeFont::GetH1Font();
//    ui->feedback_set_label->setFont(h1Font);

    ui->feedback_set_label->setProperty("type","h1");

    ui->web_feedback_btn->setProperty("type","puretextbtn");
    ui->web_feedback_btn->setText("到官网论坛反馈");


    ui->feedback_submit_btn->setProperty("type","normal");
    ui->feedback_submit_btn->setChecked(true);
    ui->insertImageBtn->setProperty("type","normal");
    ui->countLabel->setProperty("type","h2");
    ui->countLabel->setText("0/500");
    ui->textEdit->setProperty("type","noneborder");

    ui->textEdit->setPlaceholderText("  请输入问题描述及联系方式..");


    //二三候选的界面操作
    m_feedbackTypeGroup = new QButtonGroup(this);
    m_feedbackTypeGroup->addButton(ui->bug_radio_btn,0);
    m_feedbackTypeGroup->addButton(ui->improve_radio_btn,1);
    m_feedbackTypeGroup->addButton(ui->function_radio_btn,2);
    m_feedbackTypeGroup->addButton(ui->other_radio_btn,3);
    ui->bug_radio_btn->setChecked(true);
    this->setAcceptDrops(true);
}

void FeedbackStackedWidget::ConnectSignalToSlot()
{
    connect(ui->feedback_submit_btn, SIGNAL(clicked()),this,SLOT(SubmitFeedbackInfoSlot()));
    connect(m_feedbackTypeGroup, SIGNAL(buttonToggled(int, bool)), this, SLOT(ChangeFeedBackType(int,bool)));
    connect(ui->textEdit, SIGNAL(textChanged()),this, SLOT(UpdateInputCountSlot()));
    connect(ui->insertImageBtn, SIGNAL(clicked()),this, SLOT(InsertImageToTheDocument()));
    connect(ui->web_feedback_btn, SIGNAL(clicked()), this, SLOT(SlotOpenWebFeedback()));
}

void FeedbackStackedWidget::ChangeFeedBackType(int index, bool state)
{
    if(state == true)
    {
        switch(index)
        {
        case 0:
            m_typeValue = 1;
            break;
        case 1:
            m_typeValue = 2;
            break;
        case 2:
            m_typeValue = 3;
            break;
        case 3:
            m_typeValue = 4;
            break;
        }

    }
}

void FeedbackStackedWidget::SubmitFeedbackInfoSlot()
{
    QString content = ui->textEdit->toPlainText();
    QList<QString> imageNameList = ui->textEdit->GetImageNameList();


    int fileSumSize = 0;
    for(int index=0; index<imageNameList.size(); ++index)
    {
        if(QFile::exists(imageNameList.at(index)))
        {
            QFileInfo info(imageNameList.at(index));
            fileSumSize += info.size();
        }
    }
    int mb_size = fileSumSize/(1024*1024);
    if(mb_size >= 2)
    {
        MsgBoxUtils::NoticeMsgBox("文件内容超过了2M","系统提示",0,this);
        return;
    }

    ui->textEdit->setUndoRedoEnabled(false);

    if(content.isEmpty() && imageNameList.isEmpty())
    {
         MsgBoxUtils::NoticeMsgBox("反馈内容为空","系统提示",0,this);
        return;
    }
    if(content.length() > 500)
    {
        MsgBoxUtils::NoticeMsgBox("反馈内容超过500个字","系统提示",0,this);
        return;
    }
    QString title = "反馈意见";
    QString clientid = Utils::GethostMac();
    QString version =  GetImeVersion();
    QString loginid;
    ConfigManager::Instance()->GetStrConfigItem("loginid",loginid);
    QString feedbackType = QString::number(m_typeValue);
    int returnCode = UploadFeedbackInfo(version,clientid,loginid,title,content,feedbackType,imageNameList);

    if(returnCode == 0)
    {
        MsgBoxUtils::NoticeMsgBox("反馈成功","系统提示",0,this);
        ui->textEdit->clear();
        ui->countLabel->setText("0/500");
    }
    else if(returnCode == 666)
    {
        MsgBoxUtils::NoticeMsgBox("反馈超时","系统提示",0,this);
    }
    else
    {
        MsgBoxUtils::NoticeMsgBox("反馈异常","系统提示",0,this);
    }

}

void FeedbackStackedWidget::UpdateInputCountSlot()
{
    QString content = ui->textEdit->toPlainText();
    int leftWordNum = 500 - content.length();
    if(leftWordNum >= 0)
    {
         ui->countLabel->setText(QString("%1/500").arg(content.length()));
    }
    else
    {
        ui->countLabel->setText("输入超长");
    }

}


void FeedbackStackedWidget::DeleteTmpImage(int tmpImageNum)
{
    if(tmpImageNum <= 0)
    {
        return;
    }
    for(int index=0; index<tmpImageNum; ++index)
    {
        QString fileName = QString("attachList%1.png").arg(index);
        if(QFile::exists(fileName))
        {
            QFile::remove(fileName);
        }
    }
}

void FeedbackStackedWidget::InsertImageToTheDocument()
{
    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);

    QStringList fileList = QFileDialog::getOpenFileNames(this, "选择图片",
                                      desktopPath, ("所有图片(*.jpg *.png *.bmp *.tif *.tiff *.jpeg *.gif)"));
    for(int index=0; index<fileList.size(); ++index)
    {
        if(IsImageContentMatchType(fileList.at(index)))
        {
         ui->textEdit->InsertImageToTheDoc(fileList.at(index));
        }
        else
        {
            QFileInfo fileinfo(fileList.at(index));
            MsgBoxUtils::NoticeMsgBox(QString("文件格式异常\n%1").arg(fileinfo.fileName()),"系统提示",0);
        }

    }
}

void FeedbackStackedWidget::SlotOpenWebFeedback()
{

    QString default_url = ConfigManager::Instance()->GetServerUrl();
    QString feedback_url = QString("%1%2").arg(default_url).arg("/index.html#/sy?ck=yhfk");
    QDesktopServices::openUrl(QUrl(feedback_url));
}

bool FeedbackStackedWidget::IsImageContentMatchType(QString imagePath)
{
    QMap<QString,QString> file_type_header;
    file_type_header["jpg"] = "0xFFD8FF";
    file_type_header["jpeg"] = "0xFFDBEF";
    file_type_header["png"] = "0x89504E47";
    file_type_header["gif"] = "0x47494638";
    file_type_header["tif"] = "0x49492A";
    file_type_header["tiff"] = "0x49492A";
    file_type_header["bmp"] = "0x424D";

    if(!QFile::exists(imagePath))
    {
        return false;
    }

    QFileInfo file_info(imagePath);
    QString file_suffix = file_info.suffix();

    if(file_type_header.contains(file_suffix))
    {
        QFile file(imagePath);
        if(!file.open(QIODevice::ReadOnly))
        {
            return false;
        }
        QDataStream dataStream(&file);
        int length = file_type_header[file_suffix].length();
        if(file.size()<length)
        {
            return false;
        }
        char fileContent[64];
        memset(fileContent,0x00,64);
        dataStream.readRawData(fileContent,length/2 - 1);
        file.close();
        bool isOK;
        QString filetype = file_type_header[file_suffix];
        unsigned long type_header = filetype.toULong(&isOK,16);
        QString hex_str = QString(QByteArray(fileContent).toHex());
        unsigned long read_header = hex_str.toULong(&isOK,16);
        if(type_header ==  read_header)
        {
            return true;
        }
        return false;
    }
    return false;

}

QString FeedbackStackedWidget::GetImeVersion()
{
    QString version_num_str;
    QString version_file_path;

    QString install_path = TOOLS::PathUtils::GetPackageInstallpath();
    version_file_path = QDir::toNativeSeparators(install_path + QString("files/version.txt"));

    QFile version_file(version_file_path);
    if(!version_file.exists() || !version_file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        version_num_str = QString::fromLocal8Bit("");
    }
    QTextStream text_stream(&version_file);
    version_num_str = text_stream.readLine();
    version_file.close();
    if( version_num_str.isNull() || version_num_str.isEmpty())
    {
        version_num_str = QString::fromLocal8Bit("");
    }
    return version_num_str;
}

int FeedbackStackedWidget::UploadFeedbackInfo(QString version, QString clientid, QString loginid, QString title, QString content, QString type, QList<QString> imageList)
{
    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart clientidPart;
    clientidPart.setHeader(QNetworkRequest::ContentDispositionHeader,QVariant("form-data; name=\"clientid\""));
    clientidPart.setBody(QCryptographicHash::hash(clientid.toUtf8(), QCryptographicHash::Md5).toHex());
    multiPart->append(clientidPart);

    QHttpPart loginIDPart;
    loginIDPart.setHeader(QNetworkRequest::ContentDispositionHeader,QVariant("form-data; name=\"loginid\""));
    loginIDPart.setBody(loginid.toUtf8());
    multiPart->append(loginIDPart);

    QHttpPart titlePart;
    titlePart.setHeader(QNetworkRequest::ContentDispositionHeader,QVariant("form-data; name=\"title\""));
    titlePart.setBody(title.toUtf8());
    multiPart->append(titlePart);

    QHttpPart contentPart;
    contentPart.setHeader(QNetworkRequest::ContentDispositionHeader,QVariant("form-data; name=\"content\""));
    contentPart.setBody(content.toUtf8());
    multiPart->append(contentPart);

    QHttpPart typePart;
    typePart.setHeader(QNetworkRequest::ContentDispositionHeader,QVariant("form-data; name=\"type\""));
    typePart.setBody(type.toUtf8());
    multiPart->append(typePart);

    QHttpPart fromOsPart;
    fromOsPart.setHeader(QNetworkRequest::ContentDispositionHeader,QVariant("form-data; name=\"fromos\""));
    fromOsPart.setBody(QString::number(Utils::GetOSTypeIndex()).toUtf8());
    multiPart->append(fromOsPart);

    int imageIndex = 0;

    foreach(QString indexImage, imageList)
    {
        QFileInfo image_info(indexImage);
        QString file_name = image_info.fileName();
        QHttpPart attachPart;
        attachPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                             QVariant(QString("form-data; name=\"attachList\";filename=\"%1\"").arg(file_name)));

        QFile *file = new QFile(indexImage);
        file->open(QIODevice::ReadOnly);
        file->setParent(multiPart);
        attachPart.setBodyDevice(file);
        multiPart->append(attachPart);
        ++imageIndex;
    }

    QString requestUrl = ConfigManager::Instance()->GetServerUrl() + "/testapi/requesturl";

    if(!requestUrl.contains("?"))
    {
        if(!requestUrl.contains("version"))
        {
            requestUrl.append("?version=").append(Utils::GetVersion());
        }
    }
    else
    {
        if(!requestUrl.contains("version"))
        {
            requestUrl.append("&version=").append(Utils::GetVersion());
        }
    }

    QNetworkRequest request;
    request.setUrl(requestUrl);

    QString certificate_path = ":/feedback/server_certification.cer";
    QSslConfiguration config;
    QList<QSslCertificate> certs = QSslCertificate::fromPath(certificate_path,QSsl::Der);
    config.setCaCertificates(certs);
    config.setPeerVerifyMode(QSslSocket::VerifyNone);
    config.setProtocol(QSsl::TlsV1_2);
    request.setSslConfiguration(config);


    //获取错误
    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.post(request,multiPart);
    multiPart->setParent(reply);

    //添加了请求的超时操作
    QReplyTimeout *pTimeout = new QReplyTimeout(reply,5000);
    QEventLoop eventLoop;
    connect(reply, SIGNAL(finished()),&eventLoop, SLOT(quit()));
    connect(pTimeout, SIGNAL(timeout()),&eventLoop, SLOT(quit()));
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);


    if(pTimeout->GetTimeoutFlag())
    {
        return 666;
    }

    if(reply->error() != QNetworkReply::NoError)
    {
        return reply->error();
    }

    QByteArray replyData = reply->readAll();
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(replyData, &json_error));

    if(json_error.error != QJsonParseError::NoError)
    {
        return -2;
    }
    QJsonObject rootObj = jsonDoc.object();
    if(!rootObj.contains("code"))
    {
        return -3;
    }
    QString codeStr = rootObj.value("code").toString();
    int statusCode = codeStr.toInt();
    if(statusCode == 200)
    {
        return 0;
    }
    return statusCode;
}
