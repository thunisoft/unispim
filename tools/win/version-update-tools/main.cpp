#include "updatewidget.h"

#include <QApplication>
#include <QSharedMemory>
#include <QDir>
#include <Windows.h>
#include <QProcess>
#include <QTextCodec>

#include "updatecontrol.h"
#include "utils.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);
    //share segement to singleinstance
    QSharedMemory shared("huayupy-update-tools");

	Utils::WriteLogToFile(QString("version-update-tools %1").arg(a.arguments().size()));

    if (!shared.create(1) || (a.arguments().size() != 5))
	{
		QString errinfo = shared.errorString();
		Utils::WriteLogToFile(QString("Error: %1").arg(errinfo));
		return 0;
	}

    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    //load style
    QString filePath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\mainwindow.qss");
    QFile file(filePath);

    if(file.open(QIODevice::ReadOnly))
    {
        QString fileContent = file.readAll();
        a.setStyleSheet(fileContent);
        file.close();
    }
    a.setQuitOnLastWindowClosed(false);
    shared.create(256);

    Utils::WriteLogToFile(QString("download type:%1, url:%2").arg(QString(argv[2])).arg(QString(argv[1])));

    UpdateControl controler;
    controler.SetPackageurl(argv[1]);
    controler.SetDownloadType(argv[2]);
    controler.SetPackageMd5(argv[3]);
    controler.SetDownloadPackageVersion(argv[4]);

    controler.NotifyUpdate();

    return a.exec();
}
