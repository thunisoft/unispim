#include "updatewidget.h"
#include "ui_updatewidget.h"
#include "downloaddlg.h"
#include "customizemessgebox.h"
#include "utils.h"
#include <QScreen>
#include <QMessageBox>
#include <QTextCodec>
#include <Windows.h>
#include <QDir>
#include <tchar.h>
#include <QStandardPaths>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QProcess>

UpdateWidget::UpdateWidget(UpdateType updateType, QWidget *parent)
    : CustomizeQWidget(parent)
    , ui(new Ui::UpdateWidget)
    , m_updateType(updateType)
{
    ui->setupUi(this);
    InitWidget();
    SlotRefreshDPI();
}

UpdateWidget::~UpdateWidget()
{
    delete ui;
}

void UpdateWidget::InitWidget()
{
    ui->closebtn->setProperty("type","closebtn");
    ui->title_label->setProperty("type","h1");
    ui->update_now_btn->setProperty("type","normal");
    ui->update_later_btn->setProperty("type","normal");
    ui->label_2->setProperty("type","h0");
    this->setProperty("type","borderwidget");
    QPixmap pixmap(":/image/logo.png");
    ui->logo_label->setPixmap(pixmap.scaledToWidth(ui->logo_label->width()));
    m_downloadDlg = new DownloadDlg(this);
    m_customMsgBox = new CustomizeMessgeBox(this);

    this->setAttribute(Qt::WA_TranslucentBackground,true);
    if(m_updateType == NOTIFY)
    {
        ChangeToNotifyMode();
    }

    connect(ui->closebtn, SIGNAL(clicked()),this, SLOT(ExitAppSlot()));
    connect(ui->update_now_btn, SIGNAL(clicked()),this,SLOT(SlotUpdateNow()));
    connect(ui->update_later_btn,SIGNAL(clicked()), this, SLOT(SlotUpdateLater()));
    connect(m_downloadDlg, SIGNAL(exitDownload()), this, SLOT(SaveConfigAndExit()));
}

void UpdateWidget::ChangeToNotifyMode()
{
    //it's mean confirm
    ui->update_now_btn->setText(QCoreApplication::translate("UpdateWidget", "\347\241\256\345\256\232", nullptr));
    //
    ui->label_2->setText(QCoreApplication::translate("UpdateWidget", "\350\276\223\345\205\245\346\263\225\346\243\200\346\265\213\345\210\260\346\226\260\347\211\210\346\234\254,\351\207\215\345\220\257\345\220\216\346\233\264\346\226\260\343\200\202", nullptr));
}

void UpdateWidget::SlotRefreshDPI()
{
    //caculate the dpi
    QList<QScreen*> screens = QApplication::screens();
    QScreen* screen = screens[0];
    qreal dpi = screen->physicalDotsPerInch();
    //dpi = 150;
    //
    double objectRate = dpi/96.0;
    objectRate = (objectRate>1)? objectRate:1.0;
    ChangeObjectSizeAndFontSize(*this, objectRate);
    resize(width()*objectRate,height()*objectRate);
}

void UpdateWidget::ChangeObjectSizeAndFontSize(const QObject& object, double geoRate)
{
    double fontRate = geoRate;
    QList<QLabel*> resultLabel = object.findChildren<QLabel*>();
    for(int index=0; index<resultLabel.size(); ++index)
    {
        QString typeName = resultLabel.at(index)->property("type").toString();
        QLabel* currentLabel = resultLabel.at(index);
        QFont font = currentLabel->font();
        if(typeName == "h1")
        {
            int pixSize = static_cast<int>(18*fontRate);
            currentLabel->setStyleSheet(QString("QLabel{font-size:%1px;}").arg(pixSize));
        }
        else if(typeName == "h2")
        {
            font.setPixelSize(static_cast<int>(15*fontRate));
            int pixSize = static_cast<int>(15*fontRate);
            currentLabel->setFont(font);
            currentLabel->setStyleSheet(QString("QLabel{font-size:%1px;}").arg(pixSize));
        }
        else if(typeName == "main_title_label")
        {
            int pixSize = static_cast<int>(16*fontRate);
            currentLabel->setStyleSheet(QString("QLabel{font-size:%1px;}").arg(pixSize));
        }
        else if(typeName == "main_logo_label")
        {
            int width = static_cast<int>(20*geoRate);
            int height = static_cast<int>(20*geoRate);
            currentLabel->setStyleSheet(QString("QLabel{width:%1px;height:%2px;}").arg(width).arg(height));
        }
        else if(typeName == "user_logo_label")
        {
            int width = static_cast<int>(80*geoRate);
            int height = static_cast<int>(85*geoRate);
            currentLabel->setFixedSize(width,height);
        }
        else if(typeName == "greenLabel")
        {
            int pixSize = static_cast<int>(22*fontRate);
            currentLabel->setStyleSheet(QString("QLabel{font-size:%1px;}").arg(pixSize));
        }
        else if(typeName == "bluelabel")
        {
            int pixSize = static_cast<int>(14*fontRate);
            currentLabel->setStyleSheet(QString("QLabel{font-size:%1px;}").arg(pixSize));
            QFont font = currentLabel->font();
            font.setPixelSize(pixSize);
            currentLabel->setFont(font);
        }
        else if(typeName == "aboutLogoLabel")
        {
            int width = static_cast<int>(16*geoRate);
            int height = static_cast<int>(16*geoRate);
            currentLabel->setFixedSize(width,height);
        }
        else if(typeName == "copyright")
        {
            int pixSize = static_cast<int>(12*fontRate);
            currentLabel->setStyleSheet(QString("QLabel{font-size:%1px;}").arg(pixSize));
        }
        else
        {
            if(currentLabel->parent()->property("type").toString() == "window_title_widget")
            {
                font.setPixelSize(16*fontRate);
            }
            else
            {
                font.setPixelSize(14*fontRate);
            }

            currentLabel->setFont(font);
        }
    }
    QList<QPushButton*> resultBtn = object.findChildren<QPushButton*>();
    for(int index=0; index<resultBtn.size(); ++index)
    {
        QPushButton* currentBtn = resultBtn.at(index);
        QString typeName = currentBtn->property("type").toString();
        QFont font = currentBtn->font();
        QStringList normalGroup;
        normalGroup << "normal" << "puretextbtn" << "bordernormal" << "normal-radius-set-btn" << "normalfeedback";

        if(typeName == "sidebtn")
        {
            int pixSize = static_cast<int>(14*fontRate);
            int sidebtnHeight = static_cast<int>(44*geoRate);
            int paddingLeft = static_cast<int>(40*geoRate);
            //int sidebtnWidth = 150*geoRate;

            currentBtn->setStyleSheet(QString("QPushButton{font-size:%1px;height:%2px;padding-left:%3px;}").arg(pixSize).arg(sidebtnHeight).arg(paddingLeft));

        }
        else if(normalGroup.contains(typeName))
        {
            int pixSize = 14*fontRate;
            int btnWidth = 80*geoRate;
            int btnHeight = 27*geoRate;
            currentBtn->setStyleSheet(QString("QPushButton{font-size:%1px;width:%2px;height:%3px;}").arg(pixSize).arg(btnWidth).arg(btnHeight));
        }
        else if(typeName == "normal-set-btn")
        {
            int pixSize = 14*fontRate;
            int btnWidth = 120*geoRate;
            int btnHeight = 27*geoRate;
            currentBtn->setStyleSheet(QString("QPushButton{font-size:%1px;width:%2px;height:%3px;}").arg(pixSize).arg(btnWidth).arg(btnHeight));
        }
        else if(typeName == "tip")
        {
            int btnWidth = 16*geoRate;
            int btnHeight = 16*geoRate;
            currentBtn->setStyleSheet(QString("QPushButton{width:%1px;height:%2px;}").arg(btnWidth).arg(btnHeight));
        }
        else if(typeName == "closebtn" || typeName == "minbtn")
        {
            int btnWidth = 50*geoRate;
            int btnHeight = 50*geoRate;
            currentBtn->setStyleSheet(QString("QPushButton{width:%1px;height:%2px;}").arg(btnWidth).arg(btnHeight));
        }
        else if(typeName == "phraseOptionBtn")
        {
            int pixSize = 14*fontRate;
            int btnWidth = 40*geoRate;
            int btnHeight = 20*geoRate;
            currentBtn->setStyleSheet(QString("QPushButton{font-size:%1px;width:%2px;height:%3px;}").arg(pixSize).arg(btnWidth).arg(btnHeight));
        }
        else if((typeName == "loginmodebtn") || (typeName == "login") ||(typeName == "Verification_Code"))
        {
            int pixSize = 14*fontRate;
            int height = 22*geoRate;
            currentBtn->setStyleSheet(QString("QPushButton{font-size:%1px;height:%2px}").arg(pixSize).arg(height));
        }
        else if(typeName == "btngroup")
        {
            int btnWidth = 20*geoRate;
            int btnHeight = 20*geoRate;
            currentBtn->setStyleSheet(QString("QPushButton{width:%1px;height:%2px;}").arg(btnWidth).arg(btnHeight));
        }
        else if(typeName == "blueclose")
        {
            int btnWidth = 10*geoRate;
            int btnHeight = 10*geoRate;
            currentBtn->setStyleSheet(QString("QPushButton{width:%1px;height:%2px;}").arg(btnWidth).arg(btnHeight));
        }
        else if(typeName == "loginmodebtn")
        {
            int btnHeight = 44*geoRate;
            currentBtn->setStyleSheet(QString("QPushButton{width:%1px;}").arg(btnHeight));
        }
        QString objectName = currentBtn->objectName();
        QStringList objectNameList;
        objectNameList << "download_more_wordlib" << "input_wordlib_btn" << "output_wordlib_btn" << "create_wordlib_btn" << "insertImageBtn";
        if(objectNameList.contains(objectName))
        {
            int pixSize = 14*fontRate;
            int iconSize = 13*fontRate;
            currentBtn->setStyleSheet(QString("QPushButton{font-size:%1px;qproperty-iconSize:%2px %3px;}").arg(pixSize).arg(iconSize).arg(iconSize));
        }

    }

}

void UpdateWidget::SlotCloseWindow()
{
    SaveConfigAndExit();
}

void UpdateWidget::SlotUpdateNow()
{
    if(m_updateType == NOTIFY)
    {
        exit(0);
    }
    else
    {
        this->hide();
        QString file_path = GetFilePath();
        QFile file(file_path);
        if(file.exists())
        {
            QProcess::startDetached(file_path);
            SaveConfigAndExit();
        }
        else
        {
            m_downloadDlg->SetFileName(file.fileName());
            m_downloadDlg->StartDownload(m_file_addr);
            m_downloadDlg->exec();
        }
    }
}

void UpdateWidget::SlotUpdateLater()
{
    //modify the config info , then exit
    SaveConfigAndExit();
}

void UpdateWidget::SetPackageUrl(const QString url)
{
    m_file_addr = url;
}

void UpdateWidget::SaveConfigAndExit()
{
        //save the update info
        RefreshUpdateDate();
        exit(0);
}

bool UpdateWidget::RefreshUpdateDate()
{
    QString fullPath = GetConfigJsonFilePath();
    if(!QFile::exists(fullPath))
    {
        QDir dir;
        QFileInfo fileInfo(fullPath);
        QString dirPath = fileInfo.dir().path();
        dir.mkpath(dirPath);
    }

    QFile file(fullPath);
    if(!file.exists())
    {
        //配置文件不存在的时候直接创建
        Utils::WriteLogToFile("when write update time local config.json doesn't exist");
        AddUpdateTimeToEmptyFile(fullPath);
        return true;
    }

    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString value = file.readAll();
    file.close();

    if(value.isEmpty())
    {
        Utils::WriteLogToFile("config.json file content is empty");
        //配置文件内容存在但为空的时候直接添加
        AddUpdateTimeToEmptyFile(fullPath);
        return true;
    }

    QJsonParseError parseJsonErr;
    QJsonDocument document = QJsonDocument::fromJson(value.toUtf8(),&parseJsonErr);
    if(!(parseJsonErr.error == QJsonParseError::NoError))
    {
        return false;
    }
    QJsonObject jsonObject = document.object();
    QStringList keyList = jsonObject.keys();
    QDateTime dateTime(QDateTime::currentDateTime());
    QString qStr = dateTime.toString("yyyy-M-d");
    if(keyList.contains("updateTime"))
    {
        QJsonObject childObject = jsonObject.value("updateTime").toObject();
        childObject["currentvalue"] = qStr;
        jsonObject.insert("updateTime",childObject);
    }
    else
    {
        ConfigItemStruct currentObj;
        currentObj.itemName = "updateTime";
        currentObj.itemType = "string";
        currentObj.itemCurrentStrValue = qStr;
        QJsonObject JsonObj;
        JsonObj.insert("group",currentObj.itemGroupName);
        JsonObj.insert("type",currentObj.itemType);
        JsonObj.insert("currentvalue",currentObj.itemCurrentStrValue);
        JsonObj.insert("defaultvalue",currentObj.itemDefaultStrValue);
        jsonObject.insert("updateTime",JsonObj);
    }
    document.setObject(jsonObject);
    QByteArray jsonContent = document.toJson();
    QFile outputFile(fullPath);
    outputFile.open(QIODevice::WriteOnly|QIODevice::Text);
    QTextStream outputStream(&outputFile);
    outputStream << jsonContent;
    outputFile.close();
    return true;
}

QString UpdateWidget::GetFilePath()
{
    //判断返回的请求是minIO还是字节流
    QUrl url(m_file_addr);
    QRegExp urlExp(QString("^%1://.*\\?.*=.*").arg(url.scheme()));
    QString fileName;
    if(urlExp.exactMatch(m_file_addr))
    {
        QStringList partStrList = m_file_addr.split("filename=");
        if(partStrList.size() != 2)
        {
            return "";
        }
        fileName = partStrList.at(1);
    }
    else
    {
        QFileInfo fileInfo(m_file_addr);
        fileName = fileInfo.fileName();
    }

    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString fileSavePath = QString("%1/%2").arg(tempDir).arg(fileName);
    return fileSavePath;
}

QString UpdateWidget::GetConfigJsonFilePath()
{
    QString fileDir = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first().append("/AppData/Local/Thunisoft/HuayuPY/");
    QString fullPath = fileDir + QString("config.json");
    return fullPath;
}

void UpdateWidget::AddUpdateTimeToEmptyFile(QString filePath)
{
    QDateTime dateTime(QDateTime::currentDateTime());
    QString qStr = dateTime.toString("yyyy-M-d");
    ConfigItemStruct currentObj;
    currentObj.itemName = "updateTime";
    currentObj.itemType = "string";
    currentObj.itemCurrentStrValue = qStr;
    QJsonObject JsonObj;
    JsonObj.insert("group",currentObj.itemGroupName);
    JsonObj.insert("type",currentObj.itemType);
    JsonObj.insert("currentvalue",currentObj.itemCurrentStrValue);
    JsonObj.insert("defaultvalue",currentObj.itemDefaultStrValue);
    QJsonObject tempObject;
    tempObject.insert("updateTime",JsonObj);
    QJsonDocument document;

    document.setObject(tempObject);
    QByteArray jsonContent = document.toJson();
    QFile outputFile(filePath);
    outputFile.open(QIODevice::ReadWrite|QIODevice::Text);
    QTextStream outputStream(&outputFile);
    outputStream << jsonContent;
    outputFile.close();
}

void UpdateWidget::ExitAppSlot()
{
    exit(0);
}



















