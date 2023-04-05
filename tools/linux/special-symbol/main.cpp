#include "mainwindow.h"
#include <QApplication>
#include <QSharedMemory>
#include <sys/types.h>
#include <unistd.h>
#include <QDir>
#include <string.h>
#include "characterparser.h"
#include <qtsingleapplication.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        return -1;
    }

    if (strcmp(argv[1], "82717623-mhe4-0293-aduh-ku87wh6328ne") != 0)
    {
        return -1;
    }

    QtSingleApplication a(argc, argv);
    if(a.sendMessage(QString("has other process exist")))
        return 0;

    QFile file(":/qss/basicStyleSheet.qss");
    if(file.open(QIODevice::ReadOnly))
    {
        a.setStyleSheet(file.readAll());
        file.close();
    }

    MainWindow w;
    w.show();
    w.activateWindow();
    w.raise();
    return a.exec();
}
