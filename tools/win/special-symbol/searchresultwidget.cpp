#include "searchresultwidget.h"
#include "ui_searchresultwidget.h"
#include "characterwidget.h"
#include <QScrollArea>
#include <QVBoxLayout>


#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#define RESULT_COLUMN  19

SearchResultWidget::SearchResultWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SearchResultWidget)
{
    ui->setupUi(this);
    InitWidght();
}

SearchResultWidget::~SearchResultWidget()
{
    delete ui;
}

bool SearchResultWidget::UpdateSearchResult(QVector<QString> &symbolVector)
{
    int totalNum = symbolVector.size();
    ui->SearchResultLabel->setText(QString("共找到%1个结果").arg(totalNum));
    QVector<QVector<QString> > target;
    QVector<QString> lineVector;

    for(int index=0; index<totalNum; ++index)
    {
        if(lineVector.size() == RESULT_COLUMN)
        {
            target.push_back(lineVector);
            lineVector.clear();
        }
        lineVector.push_back(symbolVector.at(index));
    }
    if(lineVector.size() != 0)
    {
        target.push_back(lineVector);
    }

    m_resultWidght->SetCharacterStr(target);
    return true;
}

void SearchResultWidget::InitWidght()
{
    ui->ReturnBtn->setProperty("type","normal-set-btn");
    ui->ReturnBtn->setIcon(QIcon(":/image/returnArrow.png"));
    ui->ReturnBtn->setCursor(QCursor(Qt::PointingHandCursor));
    ui->SearchResultLabel->setStyleSheet("color:#000000;");
    ui->SearchResultLabel->setText("共找到0个结果");
    connect(ui->ReturnBtn, SIGNAL(clicked()), this, SLOT(ReturnBtnClickSlot()));

    QScrollArea* area = new QScrollArea(this);
    area->setWidgetResizable(true);

    m_resultWidght = new CharacterWidget(area);
    connect(m_resultWidght,SIGNAL(characterSelected(QString)), this, SLOT(ClickStrSlot(QString)));

    QVBoxLayout *areaLayout = new QVBoxLayout();
    areaLayout->setSpacing(30);
    areaLayout->addWidget(m_resultWidght);
    areaLayout->addStretch();
    QWidget* areaChildWidget = new QWidget();
    areaChildWidget->setProperty("type","normal-widget");
    areaChildWidget->setLayout(areaLayout);
    area->setWidget(areaChildWidget);
    QVBoxLayout* widgetLayout = new QVBoxLayout(this);
    widgetLayout->setMargin(0);
    widgetLayout->addWidget(area);
    ui->resultWidght->setLayout(widgetLayout);
}


void SearchResultWidget::ReturnBtnClickSlot()
{
    emit returnMainWindow();
}

void SearchResultWidget::ClickStrSlot(QString symbol)
{
    emit clickSearchSymbol(symbol);
}
