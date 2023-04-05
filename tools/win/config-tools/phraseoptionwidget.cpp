#include "phraseoptionwidget.h"
#include "ui_phraseoptionwidget.h"
#include "utils.h"
#include "config.h"

PhraseOptionWidget::PhraseOptionWidget(int phrase_idnex, QWidget *parent) :
    m_phrase_index(phrase_idnex),
    QWidget(parent),
    ui(new Ui::PhraseOptionWidget)
{
    Init();

    connect(ui->edit_push_button, SIGNAL(clicked(bool)), this, SLOT(OnEditButtonClicked()));
    connect(ui->delete_push_button, SIGNAL(clicked(bool)), this, SLOT(OnDeleteButtonClicked()));
}

PhraseOptionWidget::~PhraseOptionWidget()
{
    delete ui;
}


void PhraseOptionWidget::Init()
{
    ui->setupUi(this);
    double geoRate = Config::Instance()->GetGeoRate();
    int pixSize = 14*geoRate;
    int btnWidth = 40*geoRate;
    int btnHeight = 20*geoRate;
    ui->edit_push_button->setStyleSheet(QString("QPushButton{font-size:%1px;width:%2px;height:%3px;}").arg(pixSize).arg(btnWidth).arg(btnHeight));
    ui->delete_push_button->setStyleSheet(QString("QPushButton{font-size:%1px;width:%2px;height:%3px;}").arg(pixSize).arg(btnWidth).arg(btnHeight));
    ui->delete_push_button->setProperty("type","phraseOptionBtn");
    ui->edit_push_button->setProperty("type","phraseOptionBtn");
}


void PhraseOptionWidget::OnEditButtonClicked()
{
    emit OptionPhraseButtonClicked(EDIT_PHRASE, m_phrase_index);
}

void PhraseOptionWidget::OnDeleteButtonClicked()
{
//    if(!Utils::NoticeMsgBoxWithReturnValue(QString::fromLocal8Bit("确定要删除吗？"), this))
//        return;
    emit OptionPhraseButtonClicked(DELETE_PHRASE, m_phrase_index);
}
