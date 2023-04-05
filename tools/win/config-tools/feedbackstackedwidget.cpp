#include "feedbackstackedwidget.h"
#include "ui_feedbackstackedwidget.h"
#include "config.h"
#include "utils.h"
#include "networkhandler.h"
#include <QMimeData>
#include <QUrl>
#include <QList>
#include <QTextBlock>
#include <QFileDialog>
#include <QDesktopServices>
#include <QNetworkConfigurationManager>
#include <QHostInfo>

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

void FeedbackStackedWidget::ClearFeedbackMsg()
{
    ui->textEdit->clear();
    ui->countLabel->setText("0/500");
}


void FeedbackStackedWidget::InitWidget()
{
    m_typeValue = 1;
    ui->feedback_set_label->setProperty("type","h1");

    ui->feedback_submit_btn->setProperty("type","normalChecked");
    ui->insertImageBtn->setProperty("type","puretextbtn");
    ui->insertImageBtn->setLayoutDirection(Qt::LayoutDirection::LeftToRight);
    ui->insertImageBtn->setStyleSheet("text-align:left");
    ui->insertImageBtn->setIcon(QIcon(":image/insert_image.png"));
    ui->insertImageBtn->setIconSize(QSize(13,13));

    ui->feedback_on_web_btn->setProperty("type","puretextbtn");
    ui->feedback_on_web_btn->setCursor(Qt::PointingHandCursor);
    ui->feedback_submit_btn->setCursor(Qt::PointingHandCursor);
    ui->insertImageBtn->setCursor(Qt::PointingHandCursor);


    ui->countLabel->setProperty("type","h2");
    ui->countLabel->setText("0/500");
    ui->textEdit->setProperty("type","noneborder");

    ui->textEdit->setPlaceholderText("  请输入问题描述及联系方式..");

    QVector<int> labelLayout;
    bool isOk = Config::Instance()->GetLayoutVector("labellayout",labelLayout);
    if(isOk)
    {
        for(int index=0; index<labelLayout.size(); ++index)
        {
            ui->labelLayout->setStretch(index,labelLayout.at(index));

        }
    }
    //二三候选的界面操作
    m_feedbackTypeGroup = new QButtonGroup(this);
    m_feedbackTypeGroup->addButton(ui->bug_radio_btn,0);
    m_feedbackTypeGroup->addButton(ui->improve_radio_btn,1);
    m_feedbackTypeGroup->addButton(ui->function_radio_btn,2);
    m_feedbackTypeGroup->addButton(ui->other_radio_btn,3);
    ui->bug_radio_btn->setChecked(true);
    this->setAcceptDrops(true);
#ifdef BUILD_FY_VERSION
    ui->feedback_on_web_btn->setVisible(false);
#else
    ui->feedback_on_web_btn->setVisible(true);
#endif
}

void FeedbackStackedWidget::ConnectSignalToSlot()
{
    connect(ui->feedback_submit_btn, SIGNAL(clicked()),this,SLOT(SubmitFeedbackInfoSlot()));
    connect(m_feedbackTypeGroup, SIGNAL(buttonToggled(int, bool)), this, SLOT(ChangeFeedBackType(int,bool)));
    connect(ui->textEdit, SIGNAL(textChanged()),this, SLOT(UpdateInputCountSlot()));
    connect(ui->insertImageBtn, SIGNAL(clicked()),this, SLOT(InsertImageToTheDocument()));
    connect(ui->feedback_on_web_btn, SIGNAL(clicked()),this, SLOT(SlotFeedbackOnTheWeb()));
    connect(ui->textEdit, SIGNAL(ImageTypeInvalid(QString)), this, SLOT(ImageFormatErrorMsg(QString)));
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
    if(!NetworkHandler::Instance()->IsNetworkConnected())
    {
        Utils::NoticeMsgBox("网络连接异常",this);
        return;
    }
    if(!NetworkHandler::Instance()->canVisitWeb())
    {
        Utils::NoticeMsgBox("网络访问异常",this);
        return;
    }

    QString content = ui->textEdit->toPlainText();

    QRegExp reg("^(?=.*[%;&@$\\|]).*");

    if(reg.exactMatch(content))
    {
         Utils::NoticeMsgBox("反馈内容中包含特殊字符\n%;&@$|",this);
         return;
    }
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
        Utils::NoticeMsgBox("文件内容超过了2M",this);
        return;
    }

    ui->textEdit->setUndoRedoEnabled(false);

    if(content.isEmpty() && imageNameList.isEmpty())
    {
        Utils::NoticeMsgBox("反馈内容为空",this);
        return;
    }
    if(content.length() > 500)
    {
        Utils::NoticeMsgBox("反馈内容超过500个字",this);
        return;
    }
    QString title = "反馈意见";
    QString clientid = Config::Instance()->GetHostMacAddress();
    //Utils::NoticeMsgBox(QString("%1").arg(clientid));
    QString version = Config::Instance()->GetIMEVersion();
    QString loginid = Config::Instance()->GetLoginID();
    QString feedbackType = QString::number(m_typeValue);
    int returnCode = NetworkHandler::Instance()->UploadFeedbackInfo(version,clientid,loginid,title,content,feedbackType,imageNameList);

    if(returnCode == 0)
    {
        Utils::NoticeMsgBox("反馈成功",this);
        ui->textEdit->clear();
        ui->countLabel->setText("0/500");
    }
    else if(returnCode == 666)
    {
        Utils::NoticeMsgBox("反馈超时",this);
    }
    else
    {
        Utils::NoticeMsgBox("反馈异常",this);
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

void FeedbackStackedWidget::ImageFormatErrorMsg(QString fileName)
{
    Utils::NoticeMsgBox(QString("文件格式异常\n%1").arg(fileName),this);
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
            Utils::NoticeMsgBox(QString("文件格式异常\n%1").arg(fileinfo.fileName()),this);
        }

    }
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

void FeedbackStackedWidget::SlotFeedbackOnTheWeb()
{
    ConfigItemStruct configInfo;
    QString scheme;
    QString serverAddr;
    QString port;

    if(Config::Instance()->GetConfigItemByJson("server_addr",configInfo))
    {
        serverAddr = configInfo.itemCurrentStrValue;
    }
    if(Config::Instance()->GetConfigItemByJson("server_scheme",configInfo))
    {
        scheme = configInfo.itemCurrentStrValue;
    }
    if(Config::Instance()->GetConfigItemByJson("server_port", configInfo))
    {
        port = QString(":").append(QString::number(configInfo.itemCurrentIntValue));
    }

    QString completeAddr = scheme + QString("://") + serverAddr + port;
    QString feedback_page_addr = completeAddr + "/index.html#/sy?ck=yhfk";

    QDesktopServices::openUrl(QUrl(feedback_page_addr));
}

