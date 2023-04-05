#include "mainwindow.h"
#include <QApplication>
#include <QCoreApplication>
#include "checkupdateunispy.h"
#include <iostream>
#include "assert.h"

int main(int argc, char *argv[])
{

    if (argc != 3 && argc != 4)
    {
        std::cerr << "Parameter error!" << std::endl;
        return -1;
    }
    if (strcmp(argv[1], "976543-a01a-8076-872a-f5844996b1d2") != 0)
    {
        std::cerr << "Parameter error!" << std::endl;
        return -1;
    }


    if(strcmp(argv[2], UPDATE_UNISPY) != 0)
    {
        QApplication a(argc, argv);
        QString filePath = ":/style.qss";
        QFile file(filePath);
        if(file.open(QIODevice::ReadOnly))
        {
            QString fileContent = file.readAll();
            a.setStyleSheet(fileContent);
            file.close();
        }

        CheckUpdateUnispy checkUpdate;
        checkUpdate.CheckUpdate(argv[2]);
        return a.exec();
    }
    else
    {
        QApplication a(argc, argv);
        QString filePath = ":/style.qss";
        QFile file(filePath);
        if(file.open(QIODevice::ReadOnly))
        {
            QString fileContent = file.readAll();
            a.setStyleSheet(fileContent);
            file.close();
        }

        assert(QString(argv[3]).toInt() < 2);
        UpdateTipPosition position = (UpdateTipPosition)QString(argv[3]).toInt();
        MainWindow w(0, position);
        w.show();
        return a.exec();
    }

}
