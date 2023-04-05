#include <QClipboard>
#include <QDebug>

#include "config.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "customtabstyle.h"
#include "characterparser.h"
#include "tabwidget.h"
#include "inputservice.h"
#include "huayupy_adaptor.h"
#include "huayupy_interface.h"

#include <QClipboard>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QStatusBar>
#include <QHBoxLayout>
#include <QSharedMemory>
#include <QTabBar>
#include <sys/types.h>
#include <unistd.h>

#include <QDebug>
const int gRecentInputNum = 15;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);    

    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint| Qt::WindowDoesNotAcceptFocus |
                   Qt::CustomizeWindowHint |
                   Qt::Dialog);

    setAttribute(Qt::WA_QuitOnClose);

    this->setWindowIcon(QIcon(":/logo.png"));

    ui->close_btn->setProperty("type","closeBtn");

    QVector<QVector<QString> > defaultVector;
    for(int index=0; index<gRecentInputNum; ++index)
    {
        m_recentInputVector.push_back(" ");
    }
    defaultVector.push_back(m_recentInputVector);

    //QHBoxLayout *recentLayout = new QHBoxLayout(ui->recentWidget);
    m_recentWidget = new CharacterWidget();
    m_recentWidget->SetCharacterStr(defaultVector);
    //recentLayout->addWidget(m_recentWidget);
    ui->horizontalLayout->addWidget(m_recentWidget);
    ui->horizontalLayout->removeWidget(ui->recentWidget);

    setWindowTitle("符号大全");
    m_symbol_index_map.clear();
    //ui->stackedWidget->removeWidget();

    CharacterParser parser;
    bool isOK = parser.ParserFile();
    if(isOK)
    {
        QVector<QPair<QString, QString> > result = parser.GetResult();
        int size = result.size();
        for(int index=0; index<size; ++index)
        {
            QPair<QString,QString> tempPair = result.at(index);
            TabWidget* newTab = new TabWidget(this);
            QString fullStr = tempPair.first;
            QString tabName = fullStr.mid(1,fullStr.length()-2);
            newTab->SetTabName(tabName);
            newTab->SetSymbolStr(tempPair.second);
            ui->stackedWidget->addWidget(newTab);
            int tab_index = ui->stackedWidget->indexOf(newTab);
            ConnectButton(index, tab_index, tabName);
            connect(newTab,SIGNAL(SelectStr(QString)),this, SLOT(ShowRecentInputSlot(QString)));
            //ui->tabWidget->insertTab(index,newTab,tabName.toUtf8());
        }

    }
    connect(ui->clearBtn, SIGNAL(clicked()),this,SLOT(ResetRecentShowSlot()));
    connect(m_recentWidget,SIGNAL(characterSelected(QString)), this, SLOT(ShowRecentInputSlot(QString)));
    connect(ui->close_btn,&QPushButton::clicked,this, [=](){exit(0);});
    ui->title_label->setParent(this);

    InitInputService();

    UpdateHistorySymbol();

    ButtonCliked(SPECAIL_);
    ui->button_1->setChecked(true);

    new HuayupyAdaptor(this);
    new com::thunisoft::huayupy(QString(), QString(), QDBusConnection::sessionBus(), this);
    QDBusConnection::sessionBus().connect(QString(), QString(), "com.thunisoft.huayupy", "to_show", this, SLOT(ToShow()));

}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_recentWidget;
    m_recentWidget = NULL;

    UnInitInputService();
}

void MainWindow::ShowRecentInputSlot(QString inputStr)
{
    //拷贝到剪切板
    QClipboard* clipBoard = QApplication::clipboard();
    clipBoard->clear();
    clipBoard->setText(inputStr);

    this->statusBar()->clearMessage();
    this->statusBar()->showMessage(QString("符号%1已拷贝至粘贴板").arg(inputStr));

    InputService* service = GetInputServiceInstance();
    service->InputString(inputStr);

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


    Q_EMIT commit(inputStr);
    UpdateRecentShow();
}

void MainWindow::InitInputService()
{
    InputService* service = GetInputServiceInstance();
    service->Init();
}

void MainWindow::UnInitInputService()
{
    InputService* service = GetInputServiceInstance();
    service->UnInit();
    FreeInputServiceInstance(service);
}

void MainWindow::UpdateRecentShow()
{
    QVector<QVector<QString> > tempRecent;
    tempRecent.push_back(m_recentInputVector);
    m_recentWidget->SetCharacterStr(tempRecent);
}

void MainWindow::UpdateHistorySymbol()
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

void MainWindow::SaveHistorySymbol()
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

void MainWindow::ResetTheHistorySymbol(QString inputStr)
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

#define CONNECT(button_index) connect(ui->button_##button_index, SIGNAL(clicked()), this, SLOT(Button##button_index##Cliked()))
#define SET_BUTTON_NAME(button_index, name) ui->button_##button_index->setText(name)
#define SET_BUTTON_CHECKABLE(index) ui->button_##index->setCheckable(true); ui->button_##index->setAutoExclusive(true)

void MainWindow::ConnectButton(const int button_index, const int tab_index, const QString& button_name)
{
    switch ((SYMBOL)button_index) {
    case SPECAIL_:
        CONNECT(1);
        m_symbol_index_map.insert(make_pair(SPECAIL_, tab_index));
        SET_BUTTON_NAME(1, button_name);
        SET_BUTTON_CHECKABLE(1);
        break;
    case PUNC_:
        CONNECT(2);
        m_symbol_index_map.insert(make_pair(PUNC_, tab_index));
        SET_BUTTON_NAME(2, button_name);
        SET_BUTTON_CHECKABLE(2);
        break;
    case NUM_ORDER_:
        CONNECT(3);
        m_symbol_index_map.insert(make_pair(NUM_ORDER_, tab_index));
        SET_BUTTON_NAME(3, button_name);
        SET_BUTTON_CHECKABLE(3);
        break;
    case NUM_UNIT_:
        CONNECT(4);
        m_symbol_index_map.insert(make_pair(NUM_UNIT_, tab_index));
        SET_BUTTON_NAME(4, button_name);
        SET_BUTTON_CHECKABLE(4);
        break;
    case GREECE_:
        CONNECT(5);
        m_symbol_index_map.insert(make_pair(GREECE_, tab_index));
        SET_BUTTON_NAME(5, button_name);
        SET_BUTTON_CHECKABLE(5);
        break;
    case PIN_:
        CONNECT(6);
        m_symbol_index_map.insert(make_pair(PIN_, tab_index));
        SET_BUTTON_NAME(6, button_name);
        SET_BUTTON_CHECKABLE(6);
        break;
    case CN_:
        CONNECT(7);
        m_symbol_index_map.insert(make_pair(CN_, tab_index));
        SET_BUTTON_NAME(7, button_name);
        SET_BUTTON_CHECKABLE(7);
        break;
    case ENG_:
        CONNECT(8);
        m_symbol_index_map.insert(make_pair(ENG_, tab_index));
        SET_BUTTON_NAME(8, button_name);
        SET_BUTTON_CHECKABLE(8);
        break;
    case JAP_:
        CONNECT(9);
        m_symbol_index_map.insert(make_pair(JAP_, tab_index));
        SET_BUTTON_NAME(9, button_name);
        SET_BUTTON_CHECKABLE(9);
        break;
    case KOR_:
        CONNECT(10);
        m_symbol_index_map.insert(make_pair(KOR_, tab_index));
        SET_BUTTON_NAME(10, button_name);
        SET_BUTTON_CHECKABLE(10);
        break;
    case RUS_:
        CONNECT(11);
        m_symbol_index_map.insert(make_pair(RUS_, tab_index));
        SET_BUTTON_NAME(11, button_name);
        SET_BUTTON_CHECKABLE(11);
        break;
    case TAB_:
        CONNECT(12);
        m_symbol_index_map.insert(make_pair(TAB_, tab_index));
        SET_BUTTON_NAME(12, button_name);
        SET_BUTTON_CHECKABLE(12);
        break;
    default:
        break;
    }
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    SaveHistorySymbol();
    event->accept();
    exit(0);

}

void MainWindow::ResetRecentShowSlot()
{
    m_recentInputVector.clear();
    for(int index=0; index<gRecentInputNum; ++ index)
    {
        m_recentInputVector.push_back(" ");
    }
    UpdateRecentShow();
}

void MainWindow::ToShow()
{
    showNormal();
    activateWindow();
    raise();
}

void MainWindow::Button1Cliked()
{
    ButtonCliked(SPECAIL_);
}

void MainWindow::Button2Cliked()
{
    ButtonCliked(PUNC_);
}

void MainWindow::Button3Cliked()
{
    ButtonCliked(NUM_ORDER_);
}

void MainWindow::Button4Cliked()
{
    ButtonCliked(NUM_UNIT_);
}

void MainWindow::Button5Cliked()
{
    ButtonCliked(GREECE_);
}

void MainWindow::Button6Cliked()
{
    ButtonCliked(PIN_);
}

void MainWindow::Button7Cliked()
{
    ButtonCliked(CN_);
}

void MainWindow::Button8Cliked()
{
    ButtonCliked(ENG_);
}

void MainWindow::Button9Cliked()
{
    ButtonCliked(JAP_);
}

void MainWindow::Button10Cliked()
{
    ButtonCliked(KOR_);
}

void MainWindow::Button11Cliked()
{
    ButtonCliked(RUS_);
}

void MainWindow::Button12Cliked()
{
    ButtonCliked(TAB_);
}

void MainWindow::ButtonCliked(SYMBOL symbol)
{

    if(m_symbol_index_map.find(symbol) != m_symbol_index_map.end())
    {
        ui->stackedWidget->setCurrentIndex(m_symbol_index_map[symbol]);
    }

}
