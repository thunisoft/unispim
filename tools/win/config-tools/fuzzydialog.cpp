#include "fuzzydialog.h"
#include "ui_fuzzydialog.h"
#include "config.h"
#include <QScreen>
#include <QPainter>

FuzzyDialog::FuzzyDialog(QWidget *parent) :
    CustomizeQWidget(parent),
    ui(new Ui::FuzzyDialog)
{
    ui->setupUi(this);
    SetStyleSheet();
    InitWidget();
    ConnectSignalToSlot();
    InitCheckBox();
}

FuzzyDialog::~FuzzyDialog()
{
    delete ui;
}


void FuzzyDialog::InitWidget()
{
    this->setWindowFlag(Qt::WindowStaysOnTopHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    m_objectNameList.clear();
    m_checkVector.clear();
    m_objectNameList << "zh_z" << "ch_c" << "sh_s" << "k_g" << "n_l" << "r_l" << "h_f"
                     << "hu_f" << "wang_huang" << "ang_an" << "eng_en" << "ing_in";
    ui->zh_z_checkbox->setObjectName("zh_z");
    ui->sh_s_checkbox->setObjectName("sh_s");
    ui->n_l_checkbox->setObjectName("n_l");
    ui->h_f_checkbox->setObjectName("h_f");
    ui->wang_huang_checkbox->setObjectName("wang_huang");
    ui->eng_en_checkbox->setObjectName("eng_en");
    ui->ch_c_checkbox->setObjectName("ch_c");
    ui->k_g_checkbox->setObjectName("k_g");
     ui->r_l_checkbox->setObjectName("r_l");
    ui->hu_f_checkbox->setObjectName("hu_f");
    ui->ang_an_checkbox->setObjectName("ang_an");
    ui->ing_in_checkbox->setObjectName("ing_in");

    m_checkVector.push_back(ui->zh_z_checkbox);
    m_checkVector.push_back(ui->ch_c_checkbox);
    m_checkVector.push_back(ui->sh_s_checkbox);
    m_checkVector.push_back(ui->k_g_checkbox);
    m_checkVector.push_back(ui->n_l_checkbox);
     m_checkVector.push_back(ui->r_l_checkbox);
    m_checkVector.push_back(ui->h_f_checkbox);
    m_checkVector.push_back(ui->hu_f_checkbox);
    m_checkVector.push_back(ui->wang_huang_checkbox);
    m_checkVector.push_back(ui->ang_an_checkbox);
    m_checkVector.push_back(ui->eng_en_checkbox);

    m_checkVector.push_back(ui->ing_in_checkbox);

    ConfigItemStruct configInfo;
    if(Config::Instance()->GetConfigItemByJson("fuzzy_mode",configInfo))
    {
        m_initValue = configInfo.itemCurrentIntValue;
    }

    QList<QPushButton*> fuzzyBtnList;
    fuzzyBtnList.push_back(ui->reset_btn);
    fuzzyBtnList.push_back(ui->confirm_btn);
    fuzzyBtnList.push_back(ui->cancel_btn);

    for(int index=0; index<fuzzyBtnList.size();++index)
    {
        fuzzyBtnList.at(index)->setCheckable(true);
        fuzzyBtnList.at(index)->setAutoExclusive(true);
        fuzzyBtnList.at(index)->setFocusPolicy(Qt::NoFocus);
        fuzzyBtnList.at(index)->setProperty("type","normal");
        fuzzyBtnList.at(index)->setCursor(Qt::PointingHandCursor);
    }
    //ui->reset_btn->setChecked(true);

    ui->reset_btn->setCheckable(true);
    ui->reset_btn->setAutoExclusive(true);
    ui->reset_btn->setFocusPolicy(Qt::NoFocus);
    ui->reset_btn->setProperty("type","puretextbtn");

    ui->widget->setProperty("type","window_title_widget");
    ui->border_widget->setProperty("type","borderwidget");
    this->setProperty("type","borderwidget");

}

void FuzzyDialog::ConnectSignalToSlot()
{
    connect(ui->reset_btn, SIGNAL(clicked(bool)), this, SLOT(ResetSlot()));
    connect(ui->confirm_btn, SIGNAL(clicked(bool)), this, SLOT(ConfirmSlot()));
    connect(ui->cancel_btn, SIGNAL(clicked(bool)), this, SLOT(CancelSlot()));
}

void FuzzyDialog::InitCheckBox()
{
    int tempValue = m_initValue;
    for(int index=0; index<12; ++index)
    {
         if(tempValue & (1<<index))
         {
             m_checkVector.at(index)->setChecked(true);
         }
         else
         {
             m_checkVector.at(index)->setChecked(false);
         }
    }
}

void FuzzyDialog::ResetSlot()
{
    int size = m_checkVector.size();
    for(int index=0; index<size; ++index)
    {
        m_checkVector.at(index)->setChecked(false);
    }
}

void FuzzyDialog::ConfirmSlot()
{
    int size = m_checkVector.size();
    m_initValue = 0;
    for(int index=0; index<size; ++index)
    {
        if(m_checkVector.at(index)->checkState() == Qt::Checked)
        {
            QString objectStr = m_checkVector.at(index)->objectName();
            int nameIndex = m_objectNameList.indexOf(objectStr);
            if(nameIndex <0)
            {
                continue;
            }
            m_initValue |= (1 << nameIndex);
            m_initValue |= (1 << (nameIndex+12));
        }
    }
    ConfigItemStruct configInfo;
    configInfo.itemName = "fuzzy_mode";
    configInfo.itemCurrentIntValue = m_initValue;
    Config::Instance()->SetConfigItemByJson("fuzzy_mode",configInfo);
    QDialog::accept();
}

void FuzzyDialog::CancelSlot()
{
    close();
}

void FuzzyDialog::SetInitValue(const int inputValue)
{
    m_initValue = inputValue;
    InitCheckBox();
}

void FuzzyDialog::SetStyleSheet()
{

}

