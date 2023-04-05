#include "configbus.h"
#include "mainwindow.h"
#include <QApplication>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <QDesktopWidget>
#include <QDir>
#include <QTextCodec>
#include <QSharedMemory>
#include <QTranslator>
#include <QDebug>
#include <QScreen>
#include <wingdi.h>
#include "mysqlite.h"
#include "networktest.h"
#include "config.h"
#include "servicecontroller.h"
#include <QCoreApplication>
#include <QGuiApplication>
#include <Windows.h>
#include <WinUser.h>
#include <wingdi.h>


int main(int argc, char *argv[])
{
    //高分屏属性
    //QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    //QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    double dScaleFactor = 1.0;

    SetProcessDPIAware();

    HDC desktopDc = GetDC(NULL);
    float horizontal_dpi = GetDeviceCaps(desktopDc, LOGPIXELSX);
    float vrtical_dpi = GetDeviceCaps(desktopDc, LOGPIXELSY);
    int hv_dpi = (horizontal_dpi+vrtical_dpi)/2;

    dScaleFactor = static_cast<double>(hv_dpi)/96.0;


    qputenv("QT_SCALE_FACTOR", QString::number(dScaleFactor).toUtf8());

    QApplication a(argc, argv);
    a.setApplicationName("配置工具");
    a.setApplicationDisplayName("华宇拼音配置工具");

    if((a.arguments().size() == 2)&&(a.arguments().at(1) == "reloadpublishwordlib"))
    {
        Config::Instance()->LoadPublishWordlib();
        Utils::WriteLogToFile("called to reloadpublishwordlib");
        return 0;
    }

    //检测服务，服务没启动则启动服务
    ServiceController controller("HuayuPYService");
    controller.CheckTheServeice();

    //设置程序开机启动
    if((a.arguments().size() == 2)&&(a.arguments().at(1) == "autoStart"))
    {
        Utils::SetAppAutoStart();
        Utils::BackUpTheUserDefinedSpScheme();
        return 0;
    }

    if((a.arguments().size() == 3)&&(a.arguments().at(1) == "fix_addon"))
    {
        QString addon_name = a.arguments().at(2);
        int toolbarConfig =0;
        ConfigItemStruct configInfo;
        if(Config::Instance()->GetConfigItemByJson("tool_bar_config",configInfo))
        {
            toolbarConfig = configInfo.itemCurrentIntValue;
        }
        if(addon_name == "OCR")
        {
            toolbarConfig |= USE_OCR_ADDON;
        }
        else if(addon_name == "AsrInput")
        {
            toolbarConfig |= USE_VOICE_ADDON;
        }
        configInfo.itemCurrentIntValue = toolbarConfig;
        Config::Instance()->SetConfigItemByJson("tool_bar_config",configInfo);
        return 0;
    }

    //通过共享内存实现程序单例运行
    QSharedMemory shared("huayupy-config-tools");
    if(!shared.create(64))
    {
        auto set_content_to_share = [&](QString content){
            shared.lock();
            memcpy((char*)shared.data(),content.toStdString().c_str(),content.size());
            shared.unlock();
            if(shared.isAttached())
            {
                shared.detach();
            }
        };

        if(shared.attach())
        {
            //只有引擎通过特定的GUID才能显示配置工具
            if((a.arguments().size() == 2)&&(a.arguments().at(1) == "{82717623-mhe4-0293-aduh-ku87wh6328ne}"))
            {
                QString fileContent = "config-tools-show-normal";
                set_content_to_share(fileContent);
            }
            else if((a.arguments().size() == 3)&&(a.arguments().at(1) == "{82717623-mhe4-0293-aduh-ku87wh6328ne}") &&
                    (a.arguments().at(2) == "newVersionCheck"))
            {
                QString fileContent = "newVersionCheck";
                set_content_to_share(fileContent);
            }
            else if((a.arguments().size() == 3)&&(a.arguments().at(1) == "{82717623-mhe4-0293-aduh-ku87wh6328ne}") &&
                    (a.arguments().at(2) == "login_page"))
            {
                QString fileContent = "login_page";
                set_content_to_share(fileContent);
            }
            else if((a.arguments().size() == 3)&&(a.arguments().at(1) == "{82717623-mhe4-0293-aduh-ku87wh6328ne}") &&
                    (a.arguments().at(2) == "edit_hotkey"))
            {
                set_content_to_share(a.arguments().at(2));
            }

        }
        return 0;
    }

    //修改配置工具的样式
    QString filePath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\mainwindow.qss");
    QFile file(filePath);

    if(file.open(QIODevice::ReadOnly))
    {
        QString fileContent = file.readAll();
        a.setStyleSheet(fileContent);
        file.close();
    }

    QTranslator *pTranslator = new QTranslator();
    if (pTranslator->load(":/qt_zh_CN.qm"))
    {
        qApp->installTranslator(pTranslator);
    }

    MainWindow* mainwindow;
    //移动窗口到屏幕中间
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    QScreen* mainScreen = QApplication::primaryScreen();
    mainwindow = new MainWindow;
    int screenWidth = mainScreen->geometry().width();
    int screenHeight = mainScreen->geometry().height();
    mainwindow->move((screenWidth - mainwindow->width())/ 2,(screenHeight - mainwindow->height())/2);

    if(a.arguments().size() == 2)
    {
        QString callType = a.arguments().at(1);
        if(callType == "{82717623-mhe4-0293-aduh-ku87wh6328ne}")
        {
            mainwindow->show();
        }
    }
    else if(a.arguments().size() == 3)
    {
        QString callType1 = a.arguments().at(1);
        QString callType2 = a.arguments().at(2);
        if((callType1 == "{82717623-mhe4-0293-aduh-ku87wh6328ne}") && (callType2 == "newVersionCheck"))
        {
            mainwindow->show();
            mainwindow->click_index_page(ABOUT_PAGE);
        }
        else if((callType1 == "{82717623-mhe4-0293-aduh-ku87wh6328ne}") && (callType2 == "login_page"))
        {
            mainwindow->show();
            mainwindow->click_index_page(LOGIN_PAGE);
        }
    }

    int ret = a.exec();
    return ret;
}
