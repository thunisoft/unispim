#ifndef PERSONACCOUNTSTACKEDWIDGET_H
#define PERSONACCOUNTSTACKEDWIDGET_H

#include <QWidget>
#include <QScreen>
#include <QFutureWatcher>

namespace Ui {
class PersonAccountStackedWidget;
}

class PersonAccountStackedWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PersonAccountStackedWidget(QWidget *parent = nullptr);
    ~PersonAccountStackedWidget();

public:
    void SetUserState(bool isLogin);
    void UpdateUserFile();
    void ResetConfigInfo();
signals:
    void logout();
    void userconfigfileupdated();
    void userInfoHasUpdated();
    void configChanged();
    void logoutSucceed();
private:
    void InitWidget();
    void LoadConfigInfo();
    void ConnectSignalToSlot();


private slots:
    void SlotLogout();
    void SlotAccountUpdate();
    void SlotWordlibUpdate();
    int RestoreUserWordlib();
    bool IsWordlibValid(const QString& wordlib_path);

    void SlotEnableAccountUpdate(int flag);
    void SlotEnableWordlibUpdate(int flag);

    void SlotTimeComboIndexChanged(int index);

    int  SlotUpdateUserWordlibFile();

    void slot_on_userwordlib_finished();
    void slot_on_configfile_update_finished();
    void slot_on_phrasefile_update_finished();

public slots:
    int  SlotUpdateConfigFile(QString config_file_path, QString loginid);
    int  SlotUpdateCustomPhraseFile(QString phrase_file_path, QString loginid);


private:
    int LoginedUpdateUserWordlib(const QString& loginid,const QString & user_wordlib_path);
    int DownloadAndCombineUserWordlib(const QString& loginid,const QString & user_wordlib_path);
    int DownloadAndReplaceLocalUserWordlib(const QString& loginid,const QString & user_wordlib_path);


public:
    /**@brief 更新用户词库的业务函数
   * @return  函数执行结果
   * - 0 执行成功
   * - 1 执行失败
   */
    int UpdateUserWordlib(QString user_wordlib_path,QString loginid);
    int asyn_update_userwordlib();

protected:
    bool eventFilter(QObject *obj, QEvent *event);
private:
    Ui::PersonAccountStackedWidget *ui;
    QFutureWatcher<int> *m_update_userwordlib_watcher;
    QFutureWatcher<int> *m_update_userconfig_watcher;
    QFutureWatcher<int> * m_update_phrase_watcher;
};

#endif // PERSONACCOUNTSTACKEDWIDGET_H
