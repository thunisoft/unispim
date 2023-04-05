#include "wordlibshowwidget.h"
#include "ui_wordlibshowwidget.h"
#include "customize_ui/customizefont.h"
#include "../public/config.h"
#include "msgboxutils.h"
#include "wordlibnetworker.h"
#include "configbus.h"
#include <QStringList>
#include <QDir>
#include <QTableWidgetItem>
#include <QFileDialog>
#include <QFuture>
#include <QtConcurrent>

WordlibShowWidget::WordlibShowWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WordlibShowWidget)
{  
    Init();
}

WordlibShowWidget::~WordlibShowWidget()
{
    delete ui;
    if(m_wordlib_networker)
        delete m_wordlib_networker;
}

void WordlibShowWidget::Init()
{
    m_process = new QProcess(this);
    m_is_process_running = false;

    SetUpCustomizeUI();
    RegisterSlots();
    m_wordlib_count = 0;
    m_wordlib_networker = new WordlibNetworker;


    LoadAllWordlibs();
}

void WordlibShowWidget::SetUpCustomizeUI()
{
    ui->setupUi(this);

    ui->add_word_push_button->setProperty("type","normalBtn");
    ui->export_wordlib_push_button->setProperty("type","normalBtn");
    ui->clear_wordlib_push_button->setProperty("type","normalBtn");

    //ui->tip_wordlib_label_1->setFont(CustomizeFont::GetH1Font());
    ui->tip_wordlib_label_1->setProperty("type", "h1");
    ui->tip_word_label_3->setProperty("type", "h1");
    ui->tip_wordlib_label_2->setProperty("type", "h2");

    ui->tableWidget->setShowGrid(false);
    ui->tableWidget->setColumnCount(3);
    ui->tableWidget->horizontalHeader()->setVisible(false);

    ui->tableWidget->verticalHeader()->setHidden(true);
    ui->tableWidget->setFocusPolicy(Qt::NoFocus);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->tableWidget->setAlternatingRowColors(false);
    ui->tableWidget->setPalette(QColor(255, 255, 255));
    ui->tableWidget->setStyleSheet("QTableView{"
                                        "border: 1px solid #CCCCCC;"
                                        "background-color:rgb(255,255,255);"
                                        "}"
                                        "QTableView::item"
                                        "{"
                                        "padding-left:5px;"
                                        "margin-left:5px;"
                                        "border: none;"
                                        "background-color:rgb(255,255,255);"
                                        "}");
    ui->tableWidget->setColumnWidth(0,165);
    ui->tableWidget->setColumnWidth(1,165);
    ui->tableWidget->setColumnWidth(2,165);
}

void WordlibShowWidget::LoadSelectedWordlib()
{    
    //get path of ~/.config/fcitx-huayupy/wordlib uwl
    m_all_wordlib_info.clear();
    //QStringList uwl_paths = GetValidUwlFiles(Utils::GetCategoryWordlibPath());
    QStringList names = Config::Instance()->SelectedWordlibs().split(",");
    for(int i = 0; i <names.size(); i++)
    {
        if(IsExistInCloud(names[i]))
        {
            PutUwlInfoIntoRecord(Utils::GetConfigDirPath().append("wordlib/cloud/").append(names[i]),
                                 m_wordlib_selected);
            continue;
        }
        else if(IsExistInInstall(names[i]))
        {
            PutUwlInfoIntoRecord(Utils::GetInstalledWordlibPath().append(names[i]), m_wordlib_selected);
        }
        else
        {

        }
    }
    QMap<QString, WORDLIB_INFO>::iterator it = m_wordlib_selected.begin();
    for(;it != m_wordlib_selected.end(); it++)
    {
        it.value().is_selected = true;
    }
    m_all_wordlib_info.insert(SELECTED, m_wordlib_selected);
}

void WordlibShowWidget::LoadLocalCategoryWordlib()
{
    //get path of /usr/share/fcitx-huayupy/wordlib uwl
    m_wordlib_local_category.clear();
    QStringList uwl_paths = GetValidUwlFiles(Config::wordLibDirPath(WLDPL_DATA));

    foreach (QString uwl_path, uwl_paths) {
        PutUwlInfoIntoRecord(uwl_path, m_wordlib_local_category);
    }
    FilterLocalCategoryWordlib();
}

void WordlibShowWidget::FilterLocalCategoryWordlib()
{
    QMap<QString, WORDLIB_INFO>::iterator it = m_wordlib_local_category.begin();
    for(; it != m_wordlib_local_category.end(); it++)
    {
        if(m_all_wordlib_info[SELECTED].contains(it.key()))
        {
            m_wordlib_local_category[it.key()].is_selected = true;
            m_all_wordlib_info[SELECTED].remove(it.key());
        }  
    }
    m_all_wordlib_info.insert(LOCAL_CATEGORY, m_wordlib_local_category);
}

void WordlibShowWidget::FilterLocalCloudWordlib()
{
    QMap<QString, WORDLIB_INFO>::iterator it = m_wordlib_local_cloud.begin();
    QVector<QString> cloud_local_exist_vector;
    cloud_local_exist_vector.clear();

    for(; it != m_wordlib_local_cloud.end(); it++)
    {
        if(m_all_wordlib_info[SELECTED].contains(it.key()))
        {
            m_wordlib_local_cloud[it.key()].is_selected = true;
            m_all_wordlib_info[SELECTED].remove(it.key());
        }
        if(m_all_wordlib_info[LOCAL_CATEGORY].contains(it.key()))
        {
            cloud_local_exist_vector.push_back(it.key());
        }

        it.value().is_cloud_wordlib = true;
    }
    if(!cloud_local_exist_vector.isEmpty())
    {
        for(QString itemname : cloud_local_exist_vector)
        {
            if(m_wordlib_local_cloud.contains(itemname))
            {
                m_wordlib_local_cloud.remove(itemname);
            }
        }
    }

    m_all_wordlib_info.insert(LOCAL_CLOUD, m_wordlib_local_cloud);
}

void WordlibShowWidget::LoadLocalCloudWordlib()
{
    //get path of ~/.config/fcitx-huayupy/wordlib/cloud uwl
    m_wordlib_local_cloud.clear();
    QStringList uwl_paths = GetValidUwlFiles(Config::wordLibDirPath(WLDPL_CONFIG).append("cloud/"));
    foreach (QString uwl_path, uwl_paths) {
        PutUwlInfoIntoRecord(uwl_path, m_wordlib_local_cloud);
    }
    FilterLocalCloudWordlib();
}

void WordlibShowWidget::LoadRemoteWordlib()
{

    QFuture<int> wordlib_info_future = QtConcurrent::run(m_wordlib_networker,&WordlibNetworker::GetWordlibInfoFromServer);
    m_getwordlibinfo_wathcer->setFuture(wordlib_info_future);

}

QStringList WordlibShowWidget::GetValidUwlFiles(const QString& dir_path)
{
    QStringList uwl_paths;
    QDir wordlib_dir(dir_path);
    foreach(QFileInfo file_info, wordlib_dir.entryInfoList())
    {
       if (file_info.isFile() && file_info.suffix() == "uwl" && Utils::IsValidWordLibFile(file_info))
       {
           if(IsRejectedUwl(file_info.baseName()))
               continue;
            uwl_paths.push_back(file_info.absoluteFilePath());
       }
    }
    return uwl_paths;
}

void WordlibShowWidget::FillWordlibInfoIntoTableWidget(const QMap<WORDLIB_TYPE, QMap<QString, WORDLIB_INFO> >& wordlib_info)
{
    ClearTableWidget();
    QMap<WORDLIB_TYPE, QMap<QString, WORDLIB_INFO> >::const_iterator it = wordlib_info.begin();
    for(; it != wordlib_info.end(); it++)
    {
        if(it.key() == LOCAL_CATEGORY)
        {
            //default sort
            QVector<WORDLIB_INFO> temp_vector;
            QMap<QString, WORDLIB_INFO>::const_iterator begin_itor = it.value().begin();
            while(begin_itor != it.value().end())
            {
                QString file_name = begin_itor.key();
                if(DefaultCheckedWordLib(file_name))
                {
                    temp_vector.insert(temp_vector.begin(),begin_itor.value());
                }
                else
                {
                    temp_vector.push_back(begin_itor.value());
                }
                ++begin_itor;
            }
            for(int index=0; index<temp_vector.size(); ++index)
            {
                int x = m_wordlib_count / 3;
                int y = m_wordlib_count % 3;
                if(0 == y)
                {
                    int row_count = ui->tableWidget->rowCount();
                    ui->tableWidget->insertRow(row_count);
                }
                FillWordlibInfoIntoOnCell(x, y, temp_vector.at(index));
                m_wordlib_count++;
            }
        }
        else
        {
            QMap<QString, WORDLIB_INFO>::const_iterator it_2 = it.value().begin();
            for(; it_2 != it.value().end(); it_2++)
            {
                int x = m_wordlib_count / 3;
                int y = m_wordlib_count % 3;
                if(0 == y)
                {
                    int row_count = ui->tableWidget->rowCount();
                    ui->tableWidget->insertRow(row_count);
                }
                FillWordlibInfoIntoOnCell(x, y, it_2.value());
                m_wordlib_count++;
            }
        }

    }
}

void WordlibShowWidget::FillWordlibInfoIntoOnCell(int row_num, int column_num, const WORDLIB_INFO wordlib_info)
{
    WordlibCellWidget *cell_widget = AssembleOneCellWidget(wordlib_info);
    ui->tableWidget->setCellWidget(row_num, column_num, cell_widget);
    ui->tableWidget->publicUpdateGeometries();

    if (cell_widget->checkable())
    {
        if (DefaultCheckedWordLib(wordlib_info.file_name))
        {
            connect(this, SIGNAL(ResetOptionsSignal()), cell_widget, SLOT(check()));
        }
        else
        {
            connect(this, SIGNAL(ResetOptionsSignal()), cell_widget, SLOT(uncheck()));
        }
    }
}

bool WordlibShowWidget::IsRejectedUwl(const QString& uwl_base_name)
{
    if( uwl_base_name == "user" || uwl_base_name == "sys" || uwl_base_name == "syscloud" )
        return true;
    return false;
}

void WordlibShowWidget::PutUwlInfoIntoRecord(const QString& file_path, QMap<QString, WORDLIB_INFO>& container)
{
    WORDLIB_INFO wordlib_info;
    wordlib_info.is_cloud_wordlib = false;
    wordlib_info.is_selected = false;
    wordlib_info.is_local_file = true;
    if(AnalysisUwl(file_path, wordlib_info) && !container.contains(wordlib_info.file_name))
        container.insert(wordlib_info.file_name, wordlib_info);
}


bool WordlibShowWidget::AnalysisUwl(const QString& uwl_file_path, WORDLIB_INFO& wordlib_info)
{
    QFileInfo file_info(uwl_file_path);

    int length = file_info.size();

    char *buffer = new char[length];
    memset(buffer, 0, length);

    QFile file(uwl_file_path);
    if (!file.open(QFile::ReadOnly))
    {
        return false;
    }
    file.read(buffer, length);
    file.close();

    WORDLIB* wl = (WORDLIB*)buffer;
    QString wordlib_name = QString::fromUtf16(wl->header.name).toLocal8Bit();

    wordlib_info.file_path = uwl_file_path;
    wordlib_info.file_name = file_info.fileName();

    wordlib_info.wordlib_name = wordlib_name;
    wordlib_info.words_count = wl->header.word_count;
    //wordlib_info.wordlib_last_modified_date = file_info.lastModified().toString("yyyy/MM/dd");
    wordlib_info.md5 = Utils::GetFileMD5(wordlib_info.file_path);
    delete buffer;

    return true;
}



WordlibCellWidget* WordlibShowWidget::AssembleOneCellWidget(const WORDLIB_INFO& wordlib_info)
{
    WordlibCellWidget* cell_widget = new WordlibCellWidget(wordlib_info, ui->tableWidget);
    cell_widget->SetUpElements();
    if(wordlib_info.is_cloud_wordlib)
    {
        connect(cell_widget, SIGNAL(DownloadWordlib(QString)), this, SLOT(ReciveSignalToDownloadWordlib(QString)));
        connect(this, SIGNAL(WordlibDownloadDone(QString)), cell_widget, SLOT(OnDownloadDone(QString)));
    }
    return cell_widget;
}

void WordlibShowWidget::OnNetworkerGetWordlibInfoDone()
{
    disconnect(m_wordlib_networker, SIGNAL(GetWordlibInfoDone()), 0, 0);
    m_wordlib_cloud.clear();
    m_wordlib_cloud = m_wordlib_networker->GetWordlibInfo();
    m_wordlib_download_url.clear();
    m_wordlib_download_url = m_wordlib_cloud;
    if(m_wordlib_cloud.isEmpty())
        return;
    QMap<QString, WORDLIB_INFO>::iterator it = m_all_wordlib_info[LOCAL_CLOUD].begin();
    for(; it != m_all_wordlib_info[LOCAL_CLOUD].end(); it++)
    {
        if(m_wordlib_cloud.contains(it.key()))
        {
            it.value().url = m_wordlib_cloud[it.key()].url;
            m_wordlib_cloud.remove(it.key());
        }
    }
    /*如果本地词库包含同名的词库，删除之*/
    QMap<WORDLIB_TYPE, QMap<QString, WORDLIB_INFO>>::iterator it_all_wordlib
            = m_all_wordlib_info.begin();
    for(; it_all_wordlib != m_all_wordlib_info.end(); it_all_wordlib++)
    {
        QMap<QString, WORDLIB_INFO>::iterator it_type_wordlib = it_all_wordlib.value().begin();
        for(; it_type_wordlib != it_all_wordlib.value().end(); it_type_wordlib++)
        {
            if(m_wordlib_cloud.contains(it_type_wordlib.key()))
                m_wordlib_cloud.remove(it_type_wordlib.key());
        }
    }
    m_all_wordlib_info.insert(CLOUD, m_wordlib_cloud);
    FillWordlibInfoIntoTableWidget(m_all_wordlib_info);
}

void WordlibShowWidget::ClearTableWidget()
{
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);
    m_wordlib_count = 0;
}

void WordlibShowWidget::LoadAllWordlibs()
{
    ClearTableWidget();
    LoadSelectedWordlib();
    LoadLocalCategoryWordlib();
    LoadLocalCloudWordlib();
    FillWordlibInfoIntoTableWidget(m_all_wordlib_info);
    LoadRemoteWordlib();
}

void WordlibShowWidget::ReLoadWordlib()
{
    ClearTableWidget();
    m_all_wordlib_info.clear();
    m_wordlib_cloud.clear();
    m_wordlib_local_category.clear();
    m_wordlib_local_cloud.clear();
    m_wordlib_selected.clear();
    LoadSelectedWordlib();
    LoadLocalCategoryWordlib();
    FillWordlibInfoIntoTableWidget(m_all_wordlib_info);
}

void WordlibShowWidget::ReciveSignalToDownloadWordlib(const QString& wordlib_file_name)
{
    Utils::WriteLogToFile(QString("start download: %1").arg(wordlib_file_name));
    if(!m_wordlib_download_url.contains(wordlib_file_name))
    {
       Utils::WriteLogToFile(QString("wordlib_downlad_url doesn't contian%1").arg(wordlib_file_name));
        return;
    }

    if(m_wordlib_name_to_download.isEmpty())
    {
        Utils::WriteLogToFile(QString("m_wordlib_name_to_download is empty"));
        m_wordlib_name_to_download.push_back(wordlib_file_name);
        connect(m_wordlib_networker, SIGNAL(WordlibDownloadDone(int)), this, SLOT(OnDownloadWordlibDone(int)));
        WORDLIB_INFO wordlib_info = m_wordlib_download_url[m_wordlib_name_to_download.first()];
        m_wordlib_networker->DownloadWordlib(wordlib_info);
        Utils::WriteLogToFile(QString("m_wordlib_networker start download wordlib"));
    }
    else
    {

        if(!m_wordlib_name_to_download.contains(wordlib_file_name))
        {
            Utils::WriteLogToFile(QString("add wordlib to download vector"));
            m_wordlib_name_to_download.push_back(wordlib_file_name);
        }
    }
}

void WordlibShowWidget::OnDownloadWordlibDone(int state)
{
    Utils::WriteLogToFile(QString("OnDownloadWordlibDone:%1 %2").arg(state).arg(m_wordlib_name_to_download.first()));
    if(state == Ok)
        emit WordlibDownloadDone(m_wordlib_name_to_download.first());
    m_wordlib_name_to_download.pop_front();
    if(!m_wordlib_name_to_download.isEmpty())
    {
        WORDLIB_INFO wordlib_info = m_wordlib_cloud[m_wordlib_name_to_download.first()];
        m_wordlib_networker->DownloadWordlib(wordlib_info);
    }
    else
    {
        ConfigBus::instance()->valueChanged("loadWordlib", "");
        disconnect(m_wordlib_networker, SIGNAL(WordlibDownloadDone(int)), 0, 0);
    }
}

void WordlibShowWidget::OnAdduserWordlibButtonClicked()
{
    emit AddUserWordlibButtonClicked();
}

void WordlibShowWidget::RegisterSlots()
{
    m_getwordlibinfo_wathcer = new QFutureWatcher<int>(this);
    connect(m_getwordlibinfo_wathcer, &QFutureWatcher<int>::finished, this, &WordlibShowWidget::OnHandleRemoteWordlibInfo);
    connect(ui->add_word_push_button, SIGNAL(clicked()), this, SLOT(OnAdduserWordlibButtonClicked()));
    connect(ui->export_wordlib_push_button, SIGNAL(clicked()), this, SLOT(OnExportUserWordlibButtonClicked()));
    connect(ui->clear_wordlib_push_button, SIGNAL(clicked()), this, SLOT(OnClearButtonClicked()));
    connect(m_process, SIGNAL(finished(int)), this, SLOT(OnExportDone(int)));
}

void WordlibShowWidget::OnExportUserWordlibButtonClicked()
{
    if(m_is_process_running)
        return;
    m_is_process_running = true;
    QString export_path = QFileDialog::getExistingDirectory(this, QString::fromLocal8Bit("导出路径"), QDir::homePath());
    if(export_path.isEmpty())
    {
        m_is_process_running = false;
        return;
    }
    QString user_wordlib_full_path = GetUnConflictFileName(export_path);

    QStringList param;
    param << "-export";
    param << Config::Instance()->GetUserUwlPath();
    param << user_wordlib_full_path;

    CallWordlibTool(param);
}

QString WordlibShowWidget::GetUnConflictFileName(const QString& dir)
{
    QString user_wordlib_name("user_wordlib");
    QFile file(QString(dir).append("/").append(user_wordlib_name).append(".txt"));
    if(!file.exists())
    {
        m_unconflict_user_worlib_name = user_wordlib_name.append(".txt");
        return file.fileName();
    }
    int count = 0;
    while(1)
    {
        QFile file(QString(dir).append("/").append(user_wordlib_name).append(QString::number(count)).append(".txt"));
        if(!file.exists())
        {
            m_unconflict_user_worlib_name = user_wordlib_name.append(QString::number(count)).append(".txt");
            return file.fileName();
        }

        count++;
    }
}

void WordlibShowWidget::CallWordlibTool(const QStringList& param)
{
    QString process_name(Utils::GetWordlibToolPath());
    Utils::AppendFrameSuffix(process_name);
    m_process->start(process_name, param);
}

void WordlibShowWidget::OnExportDone(int exit_code)
{
    m_is_process_running = false;
    if(exit_code == 0)
    {
        MsgBoxUtils::NoticeMsgBox(QString::fromLocal8Bit("导出成功！文件名[%1]").arg(m_unconflict_user_worlib_name), "系统提示",0,this);
    }
    else
    {
        MsgBoxUtils::NoticeMsgBox(QString::fromLocal8Bit("导出失败！"), "系统提示",0, this);
    }
}

void WordlibShowWidget::OnClearButtonClicked()
{
    if(MsgBoxUtils::NoticeMsgBox(QString::fromLocal8Bit("确定要清空用户词库吗？"), "系统提示",1, this))
    {
        QFile file(Config::Instance()->GetUserUwlPath());
        ConfigBus::instance()->valueChanged("clearUserWordlib", "");
        if(file.remove() || !file.exists())
        {
            if(Config::Instance()->IsLogin())
            {
                QFile default_user_uwl(Config::Instance()->GetDefaultUserUwlPath());
                default_user_uwl.remove();
            }
            MsgBoxUtils::NoticeMsgBox(QString::fromLocal8Bit("清空成功！"), "系统提示",0, this);
            if(Config::Instance()->IsLogin())
                m_wordlib_networker->ClearUserWordlib(Config::Instance()->GetLoginId());
            ConfigBus::instance()->valueChanged("loadWordlib", "");
        }
        else
        {
            MsgBoxUtils::NoticeMsgBox(QString::fromLocal8Bit("清空失败！"), "系统提示",0, this);
        }
    }
}

void WordlibShowWidget::OnHandleRemoteWordlibInfo()
{
    if(m_getwordlibinfo_wathcer->isFinished())
    {
        QFuture<int> result = m_getwordlibinfo_wathcer->future();
        int return_code = result.resultAt(0);
        if(return_code == 0)
        {
            OnNetworkerGetWordlibInfoDone();
        }
    }
}

bool WordlibShowWidget::DefaultCheckedWordLib(const QString &file_name) const
{
    QStringList names = defaultConfig.selected_wordlibs.split(",");
    if(names.contains(file_name))
    {
        return true;
    }

    return false;
}

bool WordlibShowWidget::IsExistInCloud(const QString& name)
{
    static QString dir = Utils::GetConfigDirPath().append("wordlib/cloud/");
    QString path = dir;
    path.append(name);
    QFile file(path);
    return file.exists();
}

bool WordlibShowWidget::IsExistInInstall(const QString& name)
{
    static QString dir = Utils::GetInstalledWordlibPath();
    QString path = dir;
    path.append(name);
    QFile file(path);
    return file.exists();
}

void WordlibShowWidget::ResetOptions()
{
    emit ResetOptionsSignal();
}
