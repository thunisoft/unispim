#include "suggestsetwidget.h"
#include "../public/configmanager.h"
#include "../public/inisetting.h"
#include "configbus.h"
#include "customize_ui/customizefont.h"
#include "customize_ui/generalstyledefiner.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QSpacerItem>
#include <QApplication>
#include <QSettings>
#include <QLabel>
#include <QListView>

SuggestsetWidget::SuggestsetWidget(QWidget *parent) : CustomizeQWidget(parent)
  ,ui(new Ui::suggestset)
{
    ui->setupUi(this);
    QStringList pos;
    pos<< "1" << "2" << "3" << "4" << "5";

//   从第几个音节开始联想;
    ui->comboBox_syllable_location->setObjectName("suggestsyllablelocation");

    int suggestSyllableLocation;
    ConfigManager::Instance()->GetIntConfigItem("suggestsyllablelocation",suggestSyllableLocation);

    //m_comboBox_suggest_syllable_location->addItems(pos);
    ui->comboBox_syllable_location->addItems(pos);
    ui->comboBox_syllable_location->setCurrentIndex(suggestSyllableLocation - 1);
#ifdef USE_IBUS
    QSettings config(Config::configFilePath(), QSettings::IniFormat);
    bool candidate_count_ok = false;
    int candidateCount = config.value("basic/candidatecount").toInt(&candidate_count_ok);
    if (!candidate_count_ok) candidateCount = 8;
#else
    // Load candidate count from fcitx config file
    int candidateCount;
    ConfigManager::Instance()->GetIntConfigItem("candidates_per_line",candidateCount);
#endif
    QStringList counts;
    for(int i =1; i<candidateCount; i++)
    {
        counts << QString::number(i + 1);
    }
    //联想词在候选词中的位置
    ui->comboBox_suggestWord_location->setObjectName("suggestwordlocation");
    int suggestWordLocationNumbers;
    ConfigManager::Instance()->GetIntConfigItem("suggestwordlocation",suggestSyllableLocation);

    ui->comboBox_suggestWord_location->addItems(counts);
    ui->comboBox_suggestWord_location->setCurrentIndex(suggestWordLocationNumbers - 2);

    //联想词个数
    ui->comboBox_suggest_word_count->setObjectName("suggestwordcount");
    int suggestWordCount;
     ConfigManager::Instance()->GetIntConfigItem("suggestwordcount",suggestWordCount);

    ui->comboBox_suggest_word_count->addItems(pos);
    ui->comboBox_suggest_word_count->setCurrentIndex(suggestWordCount - 1);

    //ui->title_label->setFont(CustomizeFont::GetH1Font());
    ui->title_label->setProperty("type", "h1");
    this->setStyleSheet(QString("#suggestset { "
                        " %1 "
                        " } ").arg(GeneralStyleDefiner::BorderStyle()));

    ui->button_ok->setProperty("type","normalBtn");
    ui->button_cancel->setProperty("type","normalBtn");
    ui->button_reset->setProperty("type","normalBtn");
    ui->clostBtn->setProperty("type","closeBtn");


    connect(ui->button_ok, SIGNAL(clicked()), this, SLOT(onButtonOkClicked()));  
    connect(ui->button_reset, SIGNAL(clicked()), this, SLOT(onButtonResetClicked()));
    connect(ui->button_cancel, SIGNAL(clicked()), this, SLOT(onButtonCancelClicked()));
    connect(ui->clostBtn, SIGNAL(clicked(bool)), this, SLOT(close()));
}

void SuggestsetWidget::reset()
{
    ui->comboBox_syllable_location->setCurrentIndex(3);
    ui->comboBox_suggestWord_location->setCurrentIndex(0);
    ui->comboBox_suggest_word_count->setCurrentIndex(1);
}

void SuggestsetWidget::onButtonOkClicked()
{
    int syllabelIndex = ui->comboBox_syllable_location->currentText().toInt();
    ConfigManager::Instance()->SetIntConfigItem("suggestsyllablelocation",syllabelIndex);


    int locationIndex = ui->comboBox_suggestWord_location->currentText().toInt();;
    ConfigManager::Instance()->SetIntConfigItem("suggestwordlocation",locationIndex);


    int wordCountIndex = ui->comboBox_suggest_word_count->currentText().toInt();;
     ConfigManager::Instance()->SetIntConfigItem("suggestwordcount",wordCountIndex);


    emit CloseSuggestSetWidget();
    this->close();
}


void SuggestsetWidget::onButtonCancelClicked()
{
    this->close();
}

void SuggestsetWidget::onButtonResetClicked()
{
    reset();
}

void SuggestsetWidget::ReloadWidget()
{
    int syllabelIndex;
    ConfigManager::Instance()->GetIntConfigItem("suggestsyllablelocation",syllabelIndex);

    int locationIndex;
    ConfigManager::Instance()->GetIntConfigItem("suggestwordlocation",locationIndex);


    int wordCountIndex;
     ConfigManager::Instance()->GetIntConfigItem("suggestwordcount",wordCountIndex);

    ui->comboBox_syllable_location->setCurrentText(QString::number(syllabelIndex));
    ui->comboBox_suggestWord_location->setCurrentText(QString::number(locationIndex));
    ui->comboBox_suggest_word_count->setCurrentText(QString::number(wordCountIndex));
}
