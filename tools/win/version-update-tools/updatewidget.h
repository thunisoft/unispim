#ifndef UPDATEWIDGET_H
#define UPDATEWIDGET_H

#include "customize_qwidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class UpdateWidget; }
QT_END_NAMESPACE


struct ConfigItemStruct
{
    QString itemName;
    QString itemGroupName;
    QString itemType;
    QString itemDefaultStrValue;
    QString itemCurrentStrValue;
    int itemDefaultIntValue;
    int itemCurrentIntValue;
    ConfigItemStruct()
    {
        itemName = "";
        itemGroupName = "user";
        itemType = "int";
        itemDefaultStrValue = "";
        itemCurrentStrValue = "";
        itemCurrentIntValue = 1;
        itemDefaultIntValue = 1;
    }
};

enum UpdateType
{
    NOTIFY = 0,
    INSTALL
};

class DownloadDlg;
class CustomizeMessgeBox;
class UpdateWidget : public CustomizeQWidget
{
    Q_OBJECT

public:
    UpdateWidget(UpdateType updateType, QWidget *parent = nullptr);
    ~UpdateWidget();


private:
    void InitWidget();
    void ChangeToNotifyMode();

    QString GetFilePath();
public slots:
    void SaveConfigAndExit();

private slots:
    void ChangeObjectSizeAndFontSize(const QObject& object, double geoRate);
    void SlotRefreshDPI();

    void SlotCloseWindow();
    void SlotUpdateNow();
    void SlotUpdateLater();
    void ExitAppSlot();
public:
    void SetPackageUrl(const QString url);

private:
    bool RefreshUpdateDate();
    QString GetConfigJsonFilePath();
    void AddUpdateTimeToEmptyFile(QString filePath);



private:
    Ui::UpdateWidget *ui;
    QString m_file_addr;
    DownloadDlg* m_downloadDlg;
    CustomizeMessgeBox* m_customMsgBox;
    UpdateType m_updateType;
};
#endif // UPDATEWIDGET_H
