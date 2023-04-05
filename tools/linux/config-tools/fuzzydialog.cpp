#include "fuzzydialog.h"
#include "ui_fuzzydialog.h"
#include "customize_ui/customizefont.h"
#include "customize_ui/generalstyledefiner.h"
#include "../public/configmanager.h"

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
    m_objectNameList.clear();
    m_checkVector.clear();
    m_objectNameList << "zh_z" << "ch_c" << "sh_s" << "k_g" << "n_l" << "r_l" << "h_f"
                     << "hu_f" << "wang_huang" << "ang_an" << "eng_en" << "ing_in";
    ui->logo_label->setProperty("type", "h1");
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

    ConfigManager::Instance()->GetIntConfigItem("fuzzyconfig",m_initValue);

    ui->reset_btn->setProperty("type","normalBtn");
    ui->confirm_btn->setProperty("type","normalBtn");
    ui->cancel_btn->setProperty("type","normalBtn");
    ui->close_btn->setProperty("type","closeBtn");
}

void FuzzyDialog::ConnectSignalToSlot()
{
    connect(ui->close_btn, SIGNAL(clicked(bool)), this, SLOT(close()));
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
    ConfigManager::Instance()->SetIntConfigItem("fuzzyconfig",m_initValue);
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
    this->setStyleSheet(QString(""
                        "#FuzzyDialog  { "
                        " %1 "
                        " } "
                        "").arg(GeneralStyleDefiner::BorderStyle()));

}
