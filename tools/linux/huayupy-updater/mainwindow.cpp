#include "mainwindow.h"
#include "updatewidget.h"
#include "msgtodaemon.h"
#include "define.h"
#include "passworddialog.h"
#include <QMessageBox>
#include <QDesktopWidget>


MainWindow::MainWindow(QWidget *parent, UpdateTipPosition position) :
    QMainWindow(parent)
{
    m_updateTip = new UpdateTip(this, position);
    connect(m_updateTip, SIGNAL(SignalCancel()), this, SLOT(OnCancelButtonClicked()));
    connect(m_updateTip, SIGNAL(SignalUpdate()), this, SLOT(OnUpdateButtonClicked()));
    //setCentralWidget(m_updateWidget);
    setMinimumSize(426, 310);
    setMaximumSize(426, 310);
    setWindowTitle(QString::fromLocal8Bit("华宇拼音输入法升级程序"));
    setWindowIcon(QIcon(":/image/logo.png"));
    this ->setWindowFlags(Qt ::FramelessWindowHint);
    if(position == CENTER)
        move ((QApplication::desktop()->width() - width())/2,(QApplication::desktop()->height() - height())/2);
    else {
        move (QApplication::desktop()->width() - width(),QApplication::desktop()->height() - height() - QApplication::desktop()->availableGeometry().y());
    }
}

MainWindow::~MainWindow()
{

}

void MainWindow::closeEvent(QCloseEvent *event)
{

}

QSslConfiguration MainWindow::GetQsslConfig()
{
    QSslConfiguration config;
    QList<QSslCertificate> certs = QSslCertificate::fromPath(":/server_certification.cer",QSsl::Der);
    config.setCaCertificates(certs);
    config.setPeerVerifyMode(QSslSocket::VerifyNone);
    config.setProtocol(QSsl::TlsV1_2);
    return config;

}

void MainWindow::OnCancelButtonClicked()
{
    this->close();

}

void MainWindow::OnExit()
{
   this->close();
}

void MainWindow::OnUpdateButtonClicked()
{
    delete m_updateTip;
    m_updateTip = NULL;
    this->setVisible(false);

    QString pwd;
#ifndef USE_UOS
    PasswordDialog pwd_dlg;
    int ret = pwd_dlg.exec();
    pwd = pwd_dlg.GetPassword();
    if (ret == QDialog::Accepted)
    {
#endif
        this->setVisible(true);
        setMinimumSize(600, 346);
        setMaximumSize(600, 346);
        move ((QApplication::desktop()->width() - width())/2,(QApplication::desktop()->height() - height())/2);
        m_updateWidget = new UpdateWidget(this);
        m_updateWidget->SetPassword(pwd);
        connect(m_updateWidget, SIGNAL(to_exit()), this, SLOT(OnExit()));
        this->setFixedHeight(400);
        this->setFixedWidth(600);
        setWindowFlags(windowFlags()& ~Qt::FramelessWindowHint);
        if(!this->isVisible())
        {
            this->show();
        }
        setCentralWidget(m_updateWidget);
        emit SignalDownloadPacakge();
#ifndef USE_UOS
    }
    else
    {
        this->close();
    }
#endif
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_last_mouse_position = event->globalPos();
        m_move_widget_flag = true;
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_move_widget_flag = false;
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (!event->buttons().testFlag(Qt::LeftButton))
            return;
    if(!m_move_widget_flag)
            return;
    const QPoint position = pos() + event->globalPos() - m_last_mouse_position; //the position of mainfrmae + (current_mouse_position - last_mouse_position)
    move(position.x(), position.y());
    m_last_mouse_position = event->globalPos();
}


