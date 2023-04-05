#ifndef WORDLIBSHOWWIDGET_H
#define WORDLIBSHOWWIDGET_H

#include <QWidget>
#include <QMap>
#include <QProcess>
#include <QFutureWatcher>

#include "../public/utils.h"
#include "wordlibcellwidget.h"

class WordlibNetworker;

enum WORDLIB_TYPE
{

    LOCAL_CATEGORY = 0,
    LOCAL_CLOUD,
    CLOUD,
    SELECTED
};

namespace Ui {
class WordlibShowWidget;
}

class WordlibShowWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WordlibShowWidget(QWidget *parent = 0);
    ~WordlibShowWidget();
    void ResetOptions();

     static bool AnalysisUwl(const QString& uwl_file_path, WORDLIB_INFO& wordlib_info);

private slots:
    void OnNetworkerGetWordlibInfoDone();

    void ReciveSignalToDownloadWordlib(const QString& wordlib_file_name);

signals:
    void WordlibDownloadDone(const QString& file_name);
    void AddUserWordlibButtonClicked();
    void ResetOptionsSignal();

public slots:
    void OnDownloadWordlibDone(int state);
    void OnAdduserWordlibButtonClicked();
    void OnExportUserWordlibButtonClicked();
    void OnExportDone(int exit_code);
    void OnClearButtonClicked();
    void OnHandleRemoteWordlibInfo();
    void ReLoadWordlib();

private:
    void Init();
    void SetUpCustomizeUI();
    void RegisterSlots();
    QStringList GetValidUwlFiles(const QString& dir_path);
    void LoadAllWordlibs();
    void LoadSelectedWordlib();
    void LoadLocalCategoryWordlib();
    void LoadLocalCloudWordlib();
    void LoadRemoteWordlib();
    void FillWordlibInfoIntoTableWidget(const QMap<WORDLIB_TYPE, QMap<QString, WORDLIB_INFO> >& wordlib_info);
    void FillWordlibInfoIntoOnCell(int row_num, int column_num, const WORDLIB_INFO wordlib_info);
    bool IsRejectedUwl(const QString& uwl_base_name);
    void PutUwlInfoIntoRecord(const QString& file_path, QMap<QString, WORDLIB_INFO>& container);

    WordlibCellWidget* AssembleOneCellWidget(const WORDLIB_INFO& wordlib_info);
    void ClearTableWidget();
    void FilterLocalCategoryWordlib();
    void FilterLocalCloudWordlib();
    QString GetUnConflictFileName(const QString& dir);
    void CallWordlibTool(const QStringList& param);
    bool DefaultCheckedWordLib(const QString &file_name) const;
    bool IsExistInCloud(const QString& name);
    bool IsExistInInstall(const QString& name);

    QMap<QString, WORDLIB_INFO> m_wordlib_selected;
    QMap<QString, WORDLIB_INFO> m_wordlib_local_category;
    QMap<QString, WORDLIB_INFO> m_wordlib_local_cloud;
    QMap<QString, WORDLIB_INFO> m_wordlib_cloud;
    QMap<QString, WORDLIB_INFO> m_wordlib_download_url;
    QMap<WORDLIB_TYPE, QMap<QString, WORDLIB_INFO> >m_all_wordlib_info;
    QStringList m_wordlib_name_to_download;
    int m_wordlib_count;
    Ui::WordlibShowWidget *ui;
    WordlibNetworker* m_wordlib_networker;
    QProcess *m_process;
    QString m_unconflict_user_worlib_name;
    bool m_is_process_running;
    QFutureWatcher<int> * m_getwordlibinfo_wathcer;
};

#endif // WORDLIBSHOWWIDGET_H
