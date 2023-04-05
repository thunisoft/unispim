#include <QClipboard>
#include <QDebug>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "customtabstyle.h"
#include "characterparser.h"
#include "tabwidget.h"
#include "inputservice.h"

#include <QClipboard>
#include <QStatusBar>
#include <QHBoxLayout>
#include <QSharedMemory>
#include <sys/types.h>
#include "config.h"
#include "mainsymbolstackedwidget.h"
#include "searchfailed.h"
#include "searchresultwidget.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //主窗口的样式
    ui->setupUi(this);
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint | Qt::Tool | Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
    setAttribute(Qt::WA_QuitOnClose);
    //this->setFocusPolicy(Qt::NoFocus);
    this->setWindowIcon(QIcon(":/image/logo.ico"));
    setWindowTitle("符号大全");

    //不抢占焦点的设置
    LONG exs = GetWindowLong((HWND)this->winId(),GWL_EXSTYLE);
    exs |= WS_EX_NOACTIVATE;
    exs |= WS_EX_APPWINDOW;
    exs |= WS_EX_TOPMOST;
    exs |= WS_EX_LAYERED;
    SetWindowLong((HWND)this->winId(),GWL_EXSTYLE,exs);
    this->setFocusPolicy(Qt::ClickFocus);

    QStatusBar* statusbar = new QStatusBar(this);
    statusbar->setFixedHeight(30);
    this->setStatusBar(statusbar);
    statusbar->setSizeGripEnabled(false);

    //符号窗口的主页面
    m_mainSymbolStackedWidget = new MainSymbolStackedWidget(this);
    m_searchFailedWidget = new SearchFailed(this);
    m_searchResultWidght = new SearchResultWidget(this);

    connect(m_mainSymbolStackedWidget,SIGNAL(InputSymbol(QString)), this, SLOT(SendSymbolToOtherWindowSlot(QString)));
    connect(m_searchResultWidght, SIGNAL(clickSearchSymbol(QString)),m_mainSymbolStackedWidget,SLOT(ShowRecentInputSlot(QString)));

    ui->stackedWidget->addWidget(m_mainSymbolStackedWidget);
    ui->stackedWidget->addWidget(m_searchFailedWidget);
    ui->stackedWidget->addWidget(m_searchResultWidght);

    int initIndex = ui->stackedWidget->indexOf(m_mainSymbolStackedWidget);

    ui->stackedWidget->setCurrentIndex(initIndex);

    ui->title_widget->setProperty("type", "window_title_widget");

    m_memoryCheckThread = new SharedMemoryCheckThread();
    connect(m_memoryCheckThread, SIGNAL(ShowNormal()), this , SLOT(MainWindowShowNormalSlot()));
    m_memoryCheckThread->start();


    connect(ui->close_btn, SIGNAL(clicked()), this, SLOT(SlotCloseWindow()));
    connect(ui->min_btn, SIGNAL(clicked()), this, SLOT(SlotMinimizeTheWindow()));
    connect(ui->searchLineEdit, SIGNAL(isFocused()), this ,SLOT(SetCurrentWindowActive()));
    connect(ui->searchLineEdit, SIGNAL(returnPressed()),this, SLOT(SearchForResultSlot()));
    connect(m_searchFailedWidget, SIGNAL(returnMainWindow()), this, SLOT(SwitchToMainWindow()));
    connect(m_searchResultWidght, SIGNAL(returnMainWindow()), this, SLOT(SwitchToMainWindow()));

    InitInputService();
}

MainWindow::~MainWindow()
{
    delete ui;

    delete m_mainSymbolStackedWidget;
    m_mainSymbolStackedWidget = NULL;

    delete m_memoryCheckThread;
    m_memoryCheckThread = NULL;


    UnInitInputService();
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


void MainWindow::SlotCloseWindow()
{
    exit(0);
}

void MainWindow::SlotMinimizeTheWindow()
{
    this->showMinimized();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    event->accept();
}

void MainWindow::focusOutEvent(QFocusEvent *e)
{
    this->setWindowFlags(this->windowFlags() | Qt::WindowStaysOnTopHint);
}

void MainWindow::SendSymbolToOtherWindowSlot(QString symbol)
{
    HWND current_hwnd = GetForegroundWindow();
    if((current_hwnd == (HWND)this->winId())&&(m_preWindowHwnd != 0))
    {
        SetForegroundWindow(m_preWindowHwnd);
    }

    InputService* service = GetInputServiceInstance();
    service->InputString(symbol);
    m_mainSymbolStackedWidget->SaveHistorySymbol();
}

void MainWindow::SetCurrentWindowActive()
{
    SetForegroundWindow((HWND)this->winId());
}

void MainWindow::SearchForResultSlot()
{
    if(ui->searchLineEdit->text().isEmpty())
    {
        return;
    }
    QVector<QString> result = CharacterParser::Instance()->GetSearchResult(ui->searchLineEdit->text());
    if(result.isEmpty())
    {
        ui->stackedWidget->setCurrentWidget(m_searchFailedWidget);
    }
    else
    {
        m_searchResultWidght->UpdateSearchResult(result);
        ui->stackedWidget->setCurrentWidget(m_searchResultWidght);
    }


}

void MainWindow::SwitchToMainWindow()
{
    ui->stackedWidget->setCurrentWidget(m_mainSymbolStackedWidget);
    ui->searchLineEdit->clear();
}

void MainWindow::MainWindowShowNormalSlot()
{
    if(this->isMinimized())
    {
        this->showNormal();
    }
    if(this->isHidden())
    {
        this->showNormal();
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_last_mouse_position = event->globalPos();
        m_move_widget_flag = true;
    }

    HWND prevHwnd = GetForegroundWindow();
    if(prevHwnd != (HWND)this->winId())
    {
        m_preWindowHwnd = prevHwnd;
        SetForegroundWindow((HWND)this->winId());
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (!event->buttons().testFlag(Qt::LeftButton) || !m_move_widget_flag)
            return;
    const QPoint position = pos() + event->globalPos() - m_last_mouse_position; //the position of mainfrmae + (current_mouse_position - last_mouse_position)
    move(position.x(), position.y());
    m_last_mouse_position = event->globalPos();
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_move_widget_flag = false;
    }
}


void MainWindow::SetWindowShowOnTop()
{
    if(this->isMinimized())
    {
        this->showNormal();
    }
    ::SetWindowPos(HWND(this->winId()),HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
    ::SetWindowPos(HWND(this->winId()),HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
    this->show();

}
