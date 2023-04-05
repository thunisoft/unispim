#ifndef WORDLIBSTACKEDWIDGET_H
#define WORDLIBSTACKEDWIDGET_H

#include <QWidget>
#include <QMap>
#include <QPair>
#include <QVector>
#include "utils.h"
#include "customize_ui/basestackedwidget.h"
#include <QScreen>
#include <QModelIndex>
#include <QMouseEvent>
#include <QTableWidgetItem>
#include <QSharedPointer>
#include <QMap>
#include <QTimer>
#include <QPushButton>
#include <outputwordlibdlg.h>
#include <QButtonGroup>
#include "syswordlibcheckthread.h"
#include "updatesyswordlibthread.h"
#include "mywordlibtableview.h"
#include "networkhandler.h"


using namespace OLD_VERSION;

namespace Ui {
class WordlibStackedWidget;
}

class WordlibListInfo{
  public:
  QString fileName;
  QString viewName;
  QString download_url;
  QString type;
  QString md5;
  WordlibListInfo()
  {

  }
  ~WordlibListInfo(){}

  WordlibListInfo(QString inputfileName,QString inputViewName){
    fileName = inputfileName;
    viewName = inputViewName;
  }
  WordlibListInfo(QString inputfileName,QString inputViewName,QString inputdownload_url, QString inputtype, QString inputmd5){
    fileName = inputfileName;
    viewName = inputViewName;
    download_url = inputdownload_url;
    type = inputtype;
    md5 = inputmd5;
  }
  WordlibListInfo(const WordlibListInfo& fileinfo){
    fileName = fileinfo.fileName;
    viewName = fileinfo.viewName;
    download_url = fileinfo.download_url;
    type = fileinfo.type;
    md5 = fileinfo.md5;
  }
  WordlibListInfo& operator=(const WordlibListInfo& fileinfo){
        fileName = fileinfo.fileName;
        viewName = fileinfo.viewName;
        download_url = fileinfo.download_url;
        type = fileinfo.type;
        md5 = fileinfo.md5;
        return *this;
  }

};

enum WORDLIB_TYPE
{

    LOCAL_CATEGORY = 0,
    LOCAL_CLOUD,
    CLOUD,
    SELECTED
};
class NetworkHandler;
class ProfessWordlibCheckThread : public QThread
{

    Q_OBJECT
public:
    ProfessWordlibCheckThread() = default;
    ~ProfessWordlibCheckThread() = default;

protected:
    virtual void run() override;

signals:
    //有更新对应的信号
    void profess_wordlib_has_update();
    void profess_wordlib_has_no_update();

};

class ProfessWordlibDownloadThread : public QThread
{

    Q_OBJECT
public:
    ProfessWordlibDownloadThread() = default;
    ~ProfessWordlibDownloadThread() = default;

protected:
    virtual void run() override;

signals:
    //有更新对应的信号
    void download_finished(int error_count);
    void download_error(QString file_name);

public:
    void set_enable_flag(bool flag){m_check_enable_flag = flag;}

private:
    bool m_check_enable_flag = true;   //检查的线程
};


class InformMsgbox;
class CustomizeBtnGroup;
class OutputWordlibDlg;
class UpdateSysWordlibThread;
class WordlibStackedWidget : public BaseStackedWidget
{
    Q_OBJECT

public:
    static bool AnalysisUwl(const QString& uwl_file_path, WORDLIB_INFO& wordlib_info);


    struct ParseTxtFileResult
    {
        bool ok;
        QString errorMsg;
        QString importableTxtPath;
        ParseTxtFileResult()
        {
            ok = false;
            errorMsg = "";
            importableTxtPath = "";
        }
    };

    enum UpdateType{
      OFFICIAL_ONLY,
      CUSTOM_ONLY,
      BOTH_UPDATE
    };

    using official_wordlib_check_thread_ptr = QSharedPointer<SysWordlibCheckThread>;
    using official_wordlib_update_thread_ptr = QSharedPointer<UpdateSysWordlibThread>;
    using profess_wordlib_check_thread_ptr = QSharedPointer<ProfessWordlibCheckThread>;
    using profess_wordlib_update_thread_ptr = QSharedPointer<ProfessWordlibDownloadThread>;
    using wordlib_model = QSharedPointer<MyTableModel>;
    using wordlib_view = QSharedPointer<MyTableView>;
    using wordlib_delegate = QSharedPointer<Delegate>;
    using wordlib_info_ptr = QSharedPointer<TAB_WORDLIB_INFO>;
    using config_info = ConfigItemStruct;

public:
    explicit WordlibStackedWidget(QWidget *parent = nullptr);
    ~WordlibStackedWidget();
     void LoadConfigInfo();
     void ResetConfigInfo();
public:
     static const QStringList SYS_WORLIDB_LIST;
     QVector<WordlibListInfo> OFFICIAL_DEFAULT_WORDLIB_LIST;
     static bool combine_wordlib(QString dest_wordlib, QString wordlib_to_combine);

private:
    void init_widget();
    void init_function_map();
    void init_wordlib_thread();
    void init_wordlib_table_view();
    bool load_official_wordlib_list_from_file();

private:
    void load_wordlib();
    void load_official_wordlib();
    WordlibStackedWidget::ParseTxtFileResult parser_wordlib_text_file(QString file_path);
    bool add_wordlib_to_container(QString file_path, bool is_official = true);
    void refresh_wordlib_table_view(UpdateType update_type);

public slots:
    void slot_on_btn_clicked();
    void ReloadWordlibInfo();
    void set_check_thread_flag(bool check_falg);
    void switch_to_other_page();
    void auto_update_state_changed(int state);
    void remove_official_wordlib_slot(QString file_path);
    void remove_user_wordlib_slot(QString file_path);
    QString find_wordlibpath_by_name(QString wordlib_name);
    bool create_uwl_wordlib_from_text(const QString &txtFilePath, const QString &uwlFilePath);
    QVector<WordlibListInfo> get_official_wordlib_list();
    void slot_upate_wordlib_finished(int state);
    void slot_update_selected_wordlib();
    void slot_update_custom_wordlib_list();
    void slot_notify_wordlib_update_result();
    void slot_import_user_wordlib();
signals:
    void wordlibChanged();

private:
    void refresh_update_btn_state();

private:
    Ui::WordlibStackedWidget *ui;
    official_wordlib_check_thread_ptr m_wordlib_check_thread;  ///官方词库检查线程
    official_wordlib_update_thread_ptr m_update_wordlib_thread; ///词库更新线程
    profess_wordlib_check_thread_ptr m_profess_wordlib_check_thread; ///专业词库检查线程
    profess_wordlib_update_thread_ptr m_profess_wordlib_update_thread; ///专业词库的更新线程
    QMap<QPushButton*,std::function<void()>> m_btn_function_map; ///槽函数映射表
    QButtonGroup m_wordlib_tab_group;  ///词库模式切换模型组

    //官方词库Model-View-Delegate
    wordlib_model  m_official_wordlib_model;
    wordlib_delegate  m_official_wordlib_delegate;
    wordlib_view m_official_wordlib_view;

    //用户自定义词库Model-View-Delegate
    wordlib_model  m_user_wordlib_model;
    wordlib_delegate  m_user_wordlib_delegate;
    wordlib_view m_user_wordlib_view;

    QVector<QSharedPointer<TAB_WORDLIB_INFO>> m_offical_wordlib_info_vector;  ///官方词库信息容器
    QVector<QSharedPointer<TAB_WORDLIB_INFO>> m_user_wordlib_info_vector; ///用户自定义词库信息容器
    OutputWordlibDlg* m_outputWordlibDlg;///词库导出界面
    QVector<WordlibListInfo> m_officail_wordlib_list; ///官方词库列表
    QStringList m_custom_wordlib_list; ///用户自定义词库列表
    QVector<WordlibListInfo> m_cloud_official_wordlib_list;
    bool m_profess_wordlib_has_update = false;
    bool m_sys_wordlib_has_update = false;


};



#endif // WORDLIBSTACKEDWIDGET_H
