#include "tabwidget.h"
#include "characterwidget.h"
#include "charhandler.h"

#include <QHBoxLayout>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QDebug>

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

TabWidget::TabWidget(QWidget *parent) : QWidget(parent),
    m_symbol("")
{

}

TabWidget::~TabWidget()
{
}

void TabWidget::SetSymbolStr(const QString &symbol)
{
    m_symbol = symbol;
    InitTab();
}

void TabWidget::SetTabName(const QString &tabName)
{
    m_tabName = tabName;
    this->setProperty("tabtype",tabName);
}

void TabWidget::InitTab()
{
    this->setStyleSheet("QWidget#TabWidget{background:#FFFFFF;}");
    if(m_symbol.isEmpty())
    {
        return;
    }

    QStringList symbolList = m_symbol.split("\r\n\r\n");
    int size = symbolList.size();

    QVector<CharacterWidget*> m_charaWidget;
    m_charaWidget.clear();

    QScrollArea* area = new QScrollArea(this);
    area->setWidgetResizable(true);
    for(int index=0; index<size; ++index)
    {
        CharacterWidget* tempWidget = new CharacterWidget(area);
        connect(tempWidget,SIGNAL(characterSelected(QString)), this, SLOT(ClickStrSlot(QString)));
       if((m_tabName == "英文音符")||(m_tabName == "数字单位")||(m_tabName == "中文部首"))
       {

           CharHandler* handler = new DoubleCharHandler(symbolList.at(index));
           tempWidget->SetCharacterStr(handler->GetResult());
           m_charaWidget.push_back(tempWidget);
           delete handler;
           handler = NULL;
       }
       else if(m_tabName == "标点符号")
       {
           CharHandler* handler = new SpaceSplitHandler(symbolList.at(index));
           tempWidget->SetCharacterStr(handler->GetResult());
           m_charaWidget.push_back(tempWidget);
           delete handler;
           handler = NULL;
       }
       else
       {
           CharHandler* handler = new SingleCharHandler(symbolList.at(index));
           tempWidget->SetCharacterStr(handler->GetResult());
           m_charaWidget.push_back(tempWidget);
           delete handler;
           handler = NULL;
       }
       SetCharacterType(tempWidget,m_tabName);
    }

    QVBoxLayout *areaLayout = new QVBoxLayout();
    areaLayout->setSpacing(30);
    for(int index=0; index<m_charaWidget.size(); ++index)
    {
        areaLayout->addWidget(m_charaWidget.at(index));
    }
    areaLayout->addStretch();
    QWidget* areaChildWidget = new QWidget();
    areaChildWidget->setProperty("type","normal-widget");
    areaChildWidget->setLayout(areaLayout);
    area->setWidget(areaChildWidget);
    QVBoxLayout* widgetLayout = new QVBoxLayout(this);
    widgetLayout->setMargin(0);
    widgetLayout->addWidget(area);
    this->setLayout(widgetLayout);

}

void TabWidget::ClickStrSlot(QString clickStr)
{
    Q_EMIT SelectStr(clickStr);
}

void TabWidget::SetCharacterType(QWidget *widget, QString type)
{
    widget->setProperty("tabtype",type);
    widget->repaint();
}
