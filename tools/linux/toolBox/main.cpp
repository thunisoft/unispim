#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QDir>
#include "utils.h"
#include "config.h"
#ifdef _WIN32
#include <Windows.h>
#include <WinUser.h>
#include <wingdi.h>
#else
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <qtsingleapplication.h>
#endif



int main(int argc, char *argv[])
{
    //设置了对高分屏的适配
#ifdef _WIN32
    double dScaleFactor = 1.0;
    SetProcessDPIAware();
    HDC desktopDc = GetDC(NULL);
    float horizontal_dpi = GetDeviceCaps(desktopDc, LOGPIXELSX);
    float vrtical_dpi = GetDeviceCaps(desktopDc, LOGPIXELSY);
    int hv_dpi = (horizontal_dpi+vrtical_dpi)/2;
    dScaleFactor = static_cast<double>(hv_dpi)/96.0;
    if(dScaleFactor == 1.25)
    {
        dScaleFactor = 1;
    }
    qputenv("QT_SCALE_FACTOR", QString::number(dScaleFactor).toUtf8());

    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);
    app.setFont(QFont("Microsoft YaHei"));
    //通过共享内存实现程序单例运行
    QSharedMemory shared("huayupy-toolbox");
    if(!shared.create(64))
    {
        auto set_content_to_share = [&](QString content){
            shared.lock();
            memcpy((char*)shared.data(),content.toStdString().c_str(),content.size());
            shared.unlock();
        };
        if(shared.attach())
        {
            if(app.arguments().size() == 2)
            {
               QString pos_str = app.arguments().at(1);
               set_content_to_share(pos_str);
               Utils::set_windows_show_event();
            }
            //call single addon to enable
            else if(app.arguments().size() == 3)
            {
                if(app.arguments().at(1) == "{82717623-mhe4-0293-aduh-ku87wh6328ne}")
                {
                    set_content_to_share(app.arguments().at(2));
                    Utils::set_windows_show_event();
                }
            }
            if(shared.isAttached())
            {
                shared.detach();
            }
        }

        return 0;
    }
#else
    QtSingleApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);
    if(app.sendMessage(QString("has other process exist")))
    {
        if(app.arguments().size() == 2)
        {
            int fd;
            QString fifo_path = Config::Instance()->get_user_config_dir_path() + QString("huayupy_tool_box_fifo");
            fd = open(fifo_path.toStdString().c_str(), O_WRONLY); //等着只读
            if(fd < 0)
            {
                return 2;
            }
            QString pos_argument = app.arguments().at(1);
            char send_str[100];
            memset(send_str,0x00,sizeof(send_str));
            strcpy(send_str,pos_argument.toStdString().c_str());
            write(fd, send_str, strlen(send_str));
            close(fd);
        }
        return 0;
    }

#endif

    //添加了系统的样式表
    QString filePath = QDir::toNativeSeparators(":/toolbox_style.qss");
    QFile file(filePath);
    if(file.open(QIODevice::ReadOnly))
    {
        QString fileContent = file.readAll();
        app.setStyleSheet(fileContent);
        file.close();
    }

    if(app.arguments().size() == 2)
    {
       QString pos_str = app.arguments().at(1);
       if(pos_str.split("#").size() == 2)
       {
           QString pos_x = pos_str.split("#").at(0);
           QString pos_y = pos_str.split("#").at(1);
           int topleft_x = pos_x.toInt() - MainWindow::instance()->width();
           int topleft_y = pos_y.toInt() - MainWindow::instance()->height();
           MainWindow::instance()->move(topleft_x,topleft_y);
       }
    }
    else if(app.arguments().size() == 3)
    {
        if(app.arguments().at(1) == "{82717623-mhe4-0293-aduh-ku87wh6328ne}")
        {
            MainWindow::instance()->slot_use_addon_by_name(app.arguments().at(2));
            return 0;
        }
    }

    MainWindow::instance()->show();
    return app.exec();
}
