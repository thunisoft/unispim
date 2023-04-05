#include "mainwindow.h"
#include <QApplication>
#include <QSharedMemory>
#include <sys/types.h>
#include <QDir>
#include <string.h>
#include <QCoreApplication>
#include <QTextCodec>
#include <Windows.h>
#include <WinUser.h>
#include <wingdi.h>

#include "characterparser.h"

int main(int argc, char *argv[])
{

    //QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    double dScaleFactor = 1.0;

    SetProcessDPIAware();

    HDC desktopDc = GetDC(NULL);
    float horizontal_dpi = GetDeviceCaps(desktopDc, LOGPIXELSX);
    float vrtical_dpi = GetDeviceCaps(desktopDc, LOGPIXELSY);
    int hv_dpi = (horizontal_dpi+vrtical_dpi)/2;
    dScaleFactor = static_cast<double>(hv_dpi)/96.0;

    qputenv("QT_SCALE_FACTOR", QString::number(dScaleFactor).toUtf8());

    QApplication a(argc, argv);
    a.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));

    // 获取共享内存名称
    //通过共享内存实现单例
    QSharedMemory shared("huayupy-special-symbol");
    if(!shared.create(64))
    {
        if(shared.attach())
        {
            QString fileContent = "show normal";
            shared.lock();
            memcpy((char*)shared.data(),fileContent.toStdString().c_str(),fileContent.size());
            shared.unlock();
            if(shared.isAttached())
            {
                shared.detach();
            }
        }
        return 0;
    }

    QFile file(":/symbol-style.qss");
    QString qss_style;
    if(file.open(QIODevice::ReadOnly))
    {
        qss_style = file.readAll();
        file.close();
    }
    a.setStyleSheet(qss_style);
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    MainWindow w;
    w.show();
    return a.exec();
}
