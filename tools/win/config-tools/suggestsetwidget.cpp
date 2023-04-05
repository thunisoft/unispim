#include "suggestsetwidget.h"
#include "config.h"
#include "configbus.h"
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

    this->setAttribute(Qt::WA_TranslucentBackground,true);
    ui->label_suggestWord_location->setProperty("type","h2");
    ui->label_suggest_word_count->setProperty("type","h2");
    ui->label_syllable_location->setProperty("type","h2");

    QList<QPushButton*> suggestBtnList;
    suggestBtnList.push_back(ui->button_ok);
    suggestBtnList.push_back(ui->button_cancel);

    for(int index=0; index<suggestBtnList.size();++index)
    {
        suggestBtnList.at(index)->setCheckable(true);
        suggestBtnList.at(index)->setAutoExclusive(true);
        suggestBtnList.at(index)->setFocusPolicy(Qt::NoFocus);
        suggestBtnList.at(index)->setProperty("type","normal");
        suggestBtnList.at(index)->setCursor(Qt::PointingHandCursor);
    }

    ui->button_reset->setCheckable(true);
    ui->button_reset->setAutoExclusive(true);
    ui->button_reset->setFocusPolicy(Qt::NoFocus);
    ui->button_reset->setProperty("type","puretextbtn");
    ui->button_reset->setCursor(Qt::PointingHandCursor);

    this->setProperty("type","borderwidget");
    ui->widget->setProperty("type","window_title_widget");


    QStringList pos;
    pos<< "1" << "2" << "3" << "4" << "5";

//   从第几个音节开始联想;
    ConfigItemStruct configInfo;
    ui->comboBox_syllable_location->setObjectName("suggestsyllablelocation");
    int suggestSyllableLocation = 0;
    if(Config::Instance()->GetConfigItemByJson("suggest_syllable_location",configInfo))
    {
        suggestSyllableLocation = configInfo.itemCurrentIntValue;
    }

    ui->comboBox_syllable_location->addItems(pos);
    ui->comboBox_syllable_location->setCurrentIndex(suggestSyllableLocation - 1);
    ui->comboBox_syllable_location->setView(new QListView());
    ui->comboBox_syllable_location->setProperty("type","expandingcombo");

    // Load candidate count from fcitx config file
    int candidateCount =5;
    if(Config::Instance()->GetConfigItemByJson("candidates_per_line",configInfo))
    {
        candidateCount = configInfo.itemCurrentIntValue;
    }

    QStringList counts;
    for(int i =1; i<candidateCount; i++)
    {
        counts << QString::number(i + 1);
    }

    //联想词在候选词中的位置
    ui->comboBox_suggestWord_location->setObjectName("suggestwordlocation");
    int suggestWordLocationNumbers = 3;
    if(Config::Instance()->GetConfigItemByJson("suggest_word_location",configInfo))
    {
        suggestWordLocationNumbers = configInfo.itemCurrentIntValue;
    }

    ui->comboBox_suggestWord_location->addItems(counts);
    ui->comboBox_suggestWord_location->setCurrentIndex(suggestWordLocationNumbers - 2);
    ui->comboBox_suggestWord_location->setView(new QListView());
    ui->comboBox_suggestWord_location->setProperty("type","expandingcombo");

    //联想词个数
    ui->comboBox_suggest_word_count->setObjectName("suggestwordcount");
    int suggestWordCount = 1;
    if(Config::Instance()->GetConfigItemByJson("suggest_word_count",configInfo))
    {
        suggestWordCount = configInfo.itemCurrentIntValue;
    }


    ui->comboBox_suggest_word_count->addItems(pos);
    ui->comboBox_suggest_word_count->setCurrentIndex(suggestWordCount - 1);
    ui->comboBox_suggest_word_count->setView(new QListView());
    ui->comboBox_suggest_word_count->setProperty("type","expandingcombo");

    //event   
    connect(ui->button_ok, SIGNAL(clicked()), this, SLOT(onButtonOkClicked()));  
    connect(ui->button_reset, SIGNAL(clicked()), this, SLOT(onButtonResetClicked()));
    connect(ui->button_cancel, SIGNAL(clicked()), this, SLOT(onButtonCancelClicked()));
}

void SuggestsetWidget::reset()
{
    ui->comboBox_syllable_location->setCurrentIndex(3);
    ui->comboBox_suggestWord_location->setCurrentIndex(0);
    ui->comboBox_suggest_word_count->setCurrentIndex(1);
}

void SuggestsetWidget::onButtonOkClicked()
{
    ConfigItemStruct configInfo;
    int syllabelIndex = ui->comboBox_syllable_location->currentIndex();
    configInfo.itemName = "suggest_syllable_location";
    configInfo.itemCurrentIntValue = syllabelIndex + 1;
    Config::Instance()->SetConfigItemByJson("suggest_syllable_location",configInfo);


    configInfo.itemName = "suggest_word_location";
    int locationIndex = ui->comboBox_suggestWord_location->currentIndex();
    configInfo.itemCurrentIntValue = locationIndex+2;
    Config::Instance()->SetConfigItemByJson("suggest_word_location",configInfo);

    configInfo.itemName = "suggest_word_count";
    int wordCountIndex = ui->comboBox_suggest_word_count->currentIndex();
    configInfo.itemCurrentIntValue = wordCountIndex + 1;
    Config::Instance()->SetConfigItemByJson("suggest_word_count",configInfo);

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
    //词语联想音节位置
    ConfigItemStruct configInfo;
   if(Config::Instance()->GetConfigItemByJson("suggest_syllable_location",configInfo))
   {
       ui->comboBox_syllable_location->setCurrentIndex(configInfo.itemCurrentIntValue - 1);
   }
   else
   {
       ui->comboBox_syllable_location->setCurrentIndex(3);
   }
   //词语联想词的位置
   if(Config::Instance()->GetConfigItemByJson("suggest_word_location",configInfo))
   {
       ui->comboBox_suggestWord_location->setCurrentIndex(configInfo.itemCurrentIntValue - 2);
   }
   else
   {
       ui->comboBox_suggestWord_location->setCurrentIndex(0);
   }

   //联想词的个数
    if(Config::Instance()->GetConfigItemByJson("suggest_word_count",configInfo))
    {
         ui->comboBox_suggest_word_count->setCurrentIndex(configInfo.itemCurrentIntValue - 1);
    }
    else
    {
        ui->comboBox_suggest_word_count->setCurrentIndex(1);
    }


}
