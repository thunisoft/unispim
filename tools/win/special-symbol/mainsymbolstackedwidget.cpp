#include "mainsymbolstackedwidget.h"
#include "ui_mainsymbolstackedwidget.h"
#include "characterparser.h"
#include "customtabstyle.h"
#include "tabwidget.h"
#include "config.h"
#include <Windows.h>
#include <QTabBar>
#include "inputservice.h"

const int gRecentInputNum = 16;

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

MainSymbolStackedWidget::MainSymbolStackedWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainSymbolStackedWidget)
{
    ui->setupUi(this);
    InitWidget();
    UpdateHistorySymbol();
}

MainSymbolStackedWidget::~MainSymbolStackedWidget()
{
    delete ui;
}

void MainSymbolStackedWidget::InitWidget()
{
    //初始化主界面
    ui->label->setProperty("type","title_label");
    QFont font("Microsoft YaHei");
    font.setPixelSize(14);
    ui->label->setFont(font);
    ui->tabWidget->setTabPosition(QTabWidget::West);
    ui->tabWidget->tabBar()->setStyle(new CustomTabStyle);
    ui->tabWidget->removeTab(0);
    ui->tabWidget->removeTab(0);
    ui->clearBtn->setProperty("type","normal");
    ui->tabWidget->setDocumentMode(true);
    ui->recent_header_widght->setProperty("type","recent_header");


    //初始化最近使用的界面
    QVector<QVector<QString> > defaultVector;
    for(int index=0; index<gRecentInputNum; ++index)
    {
        m_recentInputVector.push_back(" ");
    }
    defaultVector.push_back(m_recentInputVector);

    QHBoxLayout *recentLayout = new QHBoxLayout(ui->recentWidget);
    recentLayout->setMargin(0);
    m_recentWidget = new CharacterWidget();
    m_recentWidget->SetCharacterStr(defaultVector);
    m_recentWidget->setProperty("type","recent_header");
    recentLayout->addWidget(m_recentWidget);

    //初始化所有的符号
    CharacterParser* parser = CharacterParser::Instance();
    bool isOK = parser->ParserFile();
    if(isOK)
    {
        QVector<QPair<QString, QString> > result = parser->GetResult();
        int size = result.size();
        for(int index=0; index<size; ++index)
        {
            QPair<QString,QString> tempPair = result.at(index);
            TabWidget* newTab = new TabWidget(this);
            QString fullStr = tempPair.first;
            QString tabName = fullStr.mid(1,fullStr.length()-2);
            newTab->SetTabName(tabName);
            newTab->SetSymbolStr(tempPair.second);
            connect(newTab,SIGNAL(SelectStr(QString)),this, SLOT(ShowRecentInputSlot(QString)));
            ui->tabWidget->insertTab(index,newTab,tabName);
        }

    }

    //关联信号和槽函数
    connect(ui->clearBtn, SIGNAL(clicked()),this,SLOT(ResetRecentShowSlot()));
    connect(m_recentWidget,SIGNAL(characterSelected(QString)), this, SLOT(ShowRecentInputSlot(QString)));

}


void MainSymbolStackedWidget::UpdateHistorySymbol()
{
    QVector<QString> historyVector = Config::Instance()->GetHistorySymbol();
    for(auto itor = historyVector.rbegin(); itor< historyVector.rend(); ++itor)
    {
        if((*itor).isEmpty() || (*itor) == " ")
        {
            continue;
        }
        ResetTheHistorySymbol(*itor);
    }
}

void MainSymbolStackedWidget::SaveHistorySymbol()
{
    QVector<QString> targetVector;
    for(QString index: m_recentInputVector)
    {
        if(index.isEmpty() || index == " ")
        {
            continue;
        }
        targetVector.push_back(index);
    }
    Config::Instance()->SetHistorySymbol(targetVector);
}


void MainSymbolStackedWidget::ResetTheHistorySymbol(QString inputStr)
{
    QVector<QString>::iterator begin = m_recentInputVector.begin();
    if((begin->compare(inputStr) == 0) || (inputStr == " ") || inputStr.isEmpty())
    {
        return;
    }

    if(m_recentInputVector.contains(inputStr))
    {
        QVector<QString>::iterator itor = m_recentInputVector.begin();
        while(itor != m_recentInputVector.end())
        {
            if(*itor == inputStr)
            {
                itor = m_recentInputVector.erase(itor);
            }
            else
            {
                ++itor;
            }
        }
        m_recentInputVector.insert(begin,inputStr);
    }
    else
    {
        m_recentInputVector.insert(begin,inputStr);
        m_recentInputVector.remove(m_recentInputVector.size() - 1);
    }

    UpdateRecentShow();
}

void MainSymbolStackedWidget::UpdateRecentShow()
{
    QVector<QVector<QString> > tempRecent;
    tempRecent.push_back(m_recentInputVector);
    m_recentWidget->SetCharacterStr(tempRecent);
}



void MainSymbolStackedWidget::ResetRecentShowSlot()
{
    m_recentInputVector.clear();
    for(int index=0; index<gRecentInputNum; ++ index)
    {
        m_recentInputVector.push_back(" ");
    }
    UpdateRecentShow();
}


bool MainSymbolStackedWidget::eventFilter(QObject *target, QEvent *event)
{
    if (target == ui->tabWidget)
    {
        if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease)
        {
            return true;
        }
    }
    return QWidget::eventFilter(target, event);

}


void MainSymbolStackedWidget::ShowRecentInputSlot(QString inputStr)
{
    emit InputSymbol(inputStr);

    //刷新历史记录
    QVector<QString>::iterator begin = m_recentInputVector.begin();
    if((begin->compare(inputStr) == 0) || (inputStr == " "))
    {
        return;
    }
    if(m_recentInputVector.contains(inputStr))
    {
        QVector<QString>::iterator itor = m_recentInputVector.begin();
        while(itor != m_recentInputVector.end())
        {
            if(itor == inputStr)
            {
                itor = m_recentInputVector.erase(itor);
            }
            else
            {
                ++itor;
            }
        }
       m_recentInputVector.insert(begin,inputStr);
    }
    else
    {
        m_recentInputVector.insert(begin,inputStr);
        m_recentInputVector.remove(m_recentInputVector.size() - 1);
    }

    UpdateRecentShow();
}
