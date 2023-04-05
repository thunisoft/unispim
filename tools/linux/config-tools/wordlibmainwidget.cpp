#include "wordlibmainwidget.h"
#include "ui_wordlibmainwidget.h"
#include "wordlibshowwidget.h"
#include "wordlibaddwidget.h"

WordlibMainWidget::WordlibMainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WordlibMainWidget)
{
    Init();
}

WordlibMainWidget::~WordlibMainWidget()
{
    delete ui;
}

void WordlibMainWidget::Init()
{
    SetUpCustomize();
    m_showWidget = new WordlibShowWidget(this);

    ui->stackedWidget->addWidget(m_showWidget);
    ui->stackedWidget->setCurrentIndex(2);
    ui->stackedWidget->show();
    connect(m_showWidget, SIGNAL(AddUserWordlibButtonClicked()), this, SLOT(ShowAddUserWordlibWidget()));
}

void WordlibMainWidget::SetUpCustomize()
{
    ui->setupUi(this);
}

void WordlibMainWidget::ShowShowWidget()
{
    ui->stackedWidget->setCurrentIndex(2);
}

void WordlibMainWidget::ShowAddUserWordlibWidget()
{
    if(ui->stackedWidget->count() < 4)
    {
        WordlibAddWidget *add_user_wordlib_widget = new WordlibAddWidget(this);
        ui->stackedWidget->addWidget(add_user_wordlib_widget);
        connect(add_user_wordlib_widget, SIGNAL(ReturnButtonClicked()), this, SLOT(ShowShowWidget()));
    }
    ui->stackedWidget->setCurrentIndex(3);
}

void WordlibMainWidget::ResetConfigInfo()
{
    m_showWidget->ResetOptions();
}

void WordlibMainWidget::ReloadWordInfo()
{
    m_showWidget->ReLoadWordlib();
}
