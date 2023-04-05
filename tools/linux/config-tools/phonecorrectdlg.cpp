#include "phonecorrectdlg.h"
#include "ui_phonecorrectdlg.h"
#include <QSettings>
#include "customize_ui/customizefont.h"
#include "customize_ui/generalstyledefiner.h"
#include <QDebug>
#include "../public/configmanager.h"

PhoneCorrectDlg::PhoneCorrectDlg(QWidget *parent) :
    CustomizeQWidget(parent),
    ui(new Ui::PhoneCorrectDlg),
    m_configResult(31)
{
    ui->setupUi(this);
    ui->logo_label->setProperty("type", "h1");
    setWindowTitle("拼音纠错设置");
    SetStyleSheet();
    InitCheckBox();
    ui->resetBtn->setProperty("type","normalBtn");
    ui->confirmBtn->setProperty("type","normalBtn");
    ui->cancelBtn->setProperty("type","normalBtn");
    ui->close_btn->setProperty("type","closeBtn");

    connect(ui->resetBtn, SIGNAL(clicked()), this, SLOT(onResetBtn()));
    connect(ui->cancelBtn, SIGNAL(clicked()), this, SLOT(onCancelBtn()));
    connect(ui->confirmBtn, SIGNAL(clicked()), this, SLOT(onOKBtn()));
    connect(ui->close_btn, SIGNAL(clicked()), this, SLOT(onCloseBtn()));
}

PhoneCorrectDlg::~PhoneCorrectDlg()
{
    delete ui;
}

void PhoneCorrectDlg::onOKBtn()
{
    int size = m_checkBoxVector.size();
    m_configResult = 0;
    for(int index=0; index<size; ++index)
    {
        if(m_checkBoxVector.at(index)->checkState() == Qt::Checked)
        {
            QString objectStr = m_checkBoxVector.at(index)->objectName();
            int index = m_errorList.indexOf(objectStr);
            if(index <0)
            {
                continue;
            }
            m_configResult |= (1 << index);
        }
    }
    //保存配置信息
    SaveConfig();
    emit settingChanged();
    QDialog::accept();
}

void PhoneCorrectDlg::onCancelBtn()
{
    QDialog::reject();
}

void PhoneCorrectDlg::onResetBtn()
{
    int size = m_checkBoxVector.size();
    for(int index=0; index<size; ++index)
    {
        m_checkBoxVector.at(index)->setChecked(true);
    }
}

void PhoneCorrectDlg::InitCheckBox()
{
    m_errorList << "gn"<<"mg"<<"iou"<<"uei"<<"uen";
    ui->gn_ng_Check->setObjectName("gn");
    ui->mg_ng_Check->setObjectName("mg");
    ui->iou_iu_Check->setObjectName("iou");
    ui->uei_ui_Check->setObjectName("uei");
    ui->uen_un_Check->setObjectName("uen");


    m_checkBoxVector.clear();
    m_checkBoxVector.push_back(ui->gn_ng_Check);
    m_checkBoxVector.push_back(ui->mg_ng_Check);
    m_checkBoxVector.push_back(ui->iou_iu_Check);
    m_checkBoxVector.push_back(ui->uei_ui_Check);
    m_checkBoxVector.push_back(ui->uen_un_Check);
}

int  PhoneCorrectDlg::GetCongigResult()
{
    return m_configResult;
}

void PhoneCorrectDlg::SaveConfig()
{
    ConfigManager::Instance()->SetIntConfigItem("proofread_mask",m_configResult);
}

void PhoneCorrectDlg::SetInitValue(const int inputValue)
{
    m_configResult = inputValue;
    InitCheckState();
}

void PhoneCorrectDlg::InitCheckState()
{
    int tempValue = m_configResult;
    for(int index=0; index<5; ++index)
    {
         if(tempValue & (1<<index))
         {
             m_checkBoxVector.at(index)->setChecked(true);
         }
         else
         {
             m_checkBoxVector.at(index)->setChecked(false);
         }
    }
}

void PhoneCorrectDlg::onCloseBtn()
{
    this->close();
}

void PhoneCorrectDlg::SetStyleSheet()
{
    this->setStyleSheet(QString(""
                        "#PhoneCorrectDlg  { "
                        " %1 "
                        " } "
                        "").arg(GeneralStyleDefiner::BorderStyle()));
}
