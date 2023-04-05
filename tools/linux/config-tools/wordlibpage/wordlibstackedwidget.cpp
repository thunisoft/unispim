#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QIcon>
#include <QFileDialog>
#include <QDebug>
#include <QPushButton>
#include <QDesktopServices>
#include <QDateTime>
#include <QTextStream>
#include <QTextCodec>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <memory>
#include <QNetworkConfigurationManager>

#include "wordlibstackedwidget.h"
#include "ui_wordlibstackedwidget.h"
#include "customize_ui/customizebtngroup.h"
#include "mainwindow.h"
#include "syswordlibcheckthread.h"
#include "updatesyswordlibthread.h"
#include "outputwordlibdlg.h"
#include "networkhandler.h"
#include "mywordlibtableview.h"
#include "myutils.h"
#include "../public/configmanager.h"


#ifdef QT_OS_WIN
#include "wordlibapi.h"
#include "config.h"
#include "dpiadaptor.h"
#include "WLConverter.h"
#include "utils.h"
#elif defined(Q_OS_LINUX)
#include "configbus.h"
#endif

const QStringList WordlibStackedWidget::SYS_WORLIDB_LIST = QStringList() << "sys.uwl" << "user.uwl" << "syscloud.uwl";

WordlibStackedWidget::WordlibStackedWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WordlibStackedWidget)
{
    ui->setupUi(this);
    init_widget();
    init_wordlib_thread();
    LoadConfigInfo();
}

WordlibStackedWidget::~WordlibStackedWidget()
{
    delete ui;
}


void WordlibStackedWidget::LoadConfigInfo()
{
    load_wordlib();
    ui->auto_upate_checkbox->disconnect();
#ifdef Q_OS_WIN
    ConfigItemStruct configInfo;
    if(Config::Instance()->GetConfigItemByJson("local_wordlib_network_update",configInfo))
    {
        ui->auto_upate_checkbox->setChecked(configInfo.itemCurrentIntValue);
    }
#elif defined(Q_OS_LINUX)

    int key_value;
    ConfigManager::Instance()->GetIntConfigItem("auto_update",key_value);
    ui->auto_upate_checkbox->setChecked(key_value);

#endif

    connect(ui->auto_upate_checkbox,&QCheckBox::stateChanged,this,&WordlibStackedWidget::auto_update_state_changed);
}

void WordlibStackedWidget::set_check_thread_flag(bool check_falg)
{
    if(check_falg)
    {
        m_wordlib_check_thread->start();
        m_profess_wordlib_check_thread->start();
    }
}

void WordlibStackedWidget::switch_to_other_page()
{
    QPushButton* btn = static_cast<QPushButton*>(sender());
    if(btn->objectName() == "official_wordlib_tab_btn")
    {
        ui->stackedWidget->setCurrentWidget(ui->official_wordlib_page);
    }
    else if(btn->objectName() == "user_defined_wrodlib_tab_btn")
    {
        ui->stackedWidget->setCurrentWidget(ui->user_wordlib_page);
    }
}

void WordlibStackedWidget::auto_update_state_changed(int state)
{
#ifdef Q_OS_WIN
    ConfigItemStruct configInfo;
    configInfo.itemName = "local_wordlib_network_update";
    configInfo.itemCurrentIntValue = (state == Qt::Checked)? 1:0;
    Config::Instance()->SetConfigItemByJson("local_wordlib_network_update",configInfo);
#elif defined(Q_OS_LINUX)
    int stateValue = (state == Qt::Checked)? 1:0;

    ConfigManager::Instance()->SetIntConfigItem("auto_update",stateValue);
#endif
}

void WordlibStackedWidget::remove_official_wordlib_slot(QString file_path)
{
    bool is_deleted = false;
    auto remove_official_path = [&](QSharedPointer<TAB_WORDLIB_INFO> wordlib_ptr){
        if(wordlib_ptr->file_path == file_path)
        {
            if(QFile(file_path).remove())
            {
                 is_deleted = true;
                 wordlib_ptr->current_state = CLOUD_WORDLIB;
            }
            else
            {
                is_deleted = false;
            }
        }
    };
    std::for_each(m_offical_wordlib_info_vector.begin(),m_offical_wordlib_info_vector.end(),remove_official_path);
    if(is_deleted){
        refresh_wordlib_table_view(OFFICIAL_ONLY);
        slot_update_selected_wordlib();
    }
    else
    {
        TOOLS::Messgebox::notice_msg("词库无法删除",this);
    }
}

void WordlibStackedWidget::remove_user_wordlib_slot(QString file_path)
{
    auto remove_user_path = [&](QSharedPointer<TAB_WORDLIB_INFO> wordlib_ptr){
        if(wordlib_ptr->file_path == file_path)
        {
            QFile(file_path).remove();
            m_user_wordlib_info_vector.removeOne(wordlib_ptr);
        }
    };
    std::for_each(m_user_wordlib_info_vector.begin(),m_user_wordlib_info_vector.end(),remove_user_path);
    refresh_wordlib_table_view(CUSTOM_ONLY);
    slot_update_selected_wordlib();
    slot_update_custom_wordlib_list();
}

QString WordlibStackedWidget::find_wordlibpath_by_name(QString wordlib_name)
{
    for(int index = 0; index<m_offical_wordlib_info_vector.size(); ++index)
    {
        if(m_offical_wordlib_info_vector.at(index)->wordlib_name == wordlib_name)
        {
            return m_offical_wordlib_info_vector.at(index)->file_path;
        }
    }

    for(int index=0; index<m_user_wordlib_info_vector.size(); ++index)
    {
        if(m_user_wordlib_info_vector.at(index)->wordlib_name == wordlib_name)
        {
            return m_user_wordlib_info_vector.at(index)->file_path;
        }
    }
    return QString();
}



void WordlibStackedWidget::init_widget()
{

    QList<QPushButton*> all_btn;
    all_btn << ui->user_wordlib_export_btn << ui->user_wordlib_import_btn << ui->clear_user_wordlib_btn
            << ui->export_category_wordlib_btn << ui->import_category_wordlib_btn;

    ui->export_category_wordlib_btn->setVisible(false);
    ui->import_category_wordlib_btn->setVisible(false);

    ui->update_category_wordlib_btn->setProperty("type","puretextbtn");
    ui->update_category_wordlib_btn->setCursor(Qt::PointingHandCursor);
    connect(ui->update_category_wordlib_btn,&QPushButton::clicked,this,&WordlibStackedWidget::slot_on_btn_clicked);

    //设置所有按键的样式
    auto set_btn_style = [this](QPushButton*input_btn){
        input_btn->setProperty("type","normal");
        input_btn->setCursor(Qt::PointingHandCursor);
        connect(input_btn,&QPushButton::clicked,this,&WordlibStackedWidget::slot_on_btn_clicked);
    };
    std::for_each(all_btn.begin(), all_btn.end(),set_btn_style);

    //设置label的样式
    QList<QLabel*> h1_style_list;
    h1_style_list << ui->category_wordlib_label << ui->wordlib_set_label;
    auto set_h1_label = [this](QLabel* label){
        label->setProperty("type","h1");
    };
    std::for_each(h1_style_list.begin(),h1_style_list.end(),set_h1_label);
    ui->chinese_wordlib_label->setProperty("type","h2");

    //设置页签按钮的样式
    QList<QPushButton*> wordlib_tab_list;
    wordlib_tab_list << ui->user_defined_wrodlib_tab_btn << ui->official_wordlib_tab_btn;
    auto set_tab_style = [this](QPushButton* btn){
        btn->setProperty("type","wordlib_tab");
        btn->setCheckable(true);
        btn->setCursor(Qt::PointingHandCursor);
        m_wordlib_tab_group.addButton(btn);
    };
    std::for_each(wordlib_tab_list.begin(),wordlib_tab_list.end(),set_tab_style);
    ui->official_wordlib_tab_btn->setChecked(true);
    ui->wordlib_table_widget->setProperty("type","wordlib_table");

    //导出词库窗口
    m_outputWordlibDlg = new OutputWordlibDlg(this);
    connect(ui->official_wordlib_tab_btn,&QPushButton::clicked,this,&WordlibStackedWidget::switch_to_other_page);
    connect(ui->user_defined_wrodlib_tab_btn,&QPushButton::clicked,this,&WordlibStackedWidget::switch_to_other_page);

    init_wordlib_table_view();
    ui->stackedWidget->setCurrentWidget(ui->official_wordlib_page);

}


void WordlibStackedWidget::init_wordlib_thread()
{
    m_wordlib_check_thread = QSharedPointer<SysWordlibCheckThread>(new SysWordlibCheckThread());
    m_profess_wordlib_check_thread = QSharedPointer<ProfessWordlibCheckThread>(new ProfessWordlibCheckThread());

    connect(m_wordlib_check_thread.data(),&SysWordlibCheckThread::updatecoming,this,[&](QString version){
        m_sys_wordlib_has_update = true;
        refresh_update_btn_state();});
    connect(m_wordlib_check_thread.data(),&SysWordlibCheckThread::nowordlibupdate,this,[&](){
        m_sys_wordlib_has_update = false;
        refresh_update_btn_state();
       });
    connect(m_wordlib_check_thread.data(), &SysWordlibCheckThread::current_date_version_update,this,[&](QString date){
        ui->sys_wordlib_version_label->setText(QString("当前系统词库更新日期:%1").arg(date));
    });

    connect(m_profess_wordlib_check_thread.data(),&ProfessWordlibCheckThread::profess_wordlib_has_update,this,[&](){
       m_profess_wordlib_has_update = true;
       refresh_update_btn_state();
    });
    connect(m_profess_wordlib_check_thread.data(),&ProfessWordlibCheckThread::profess_wordlib_has_no_update,this,[&](){
       m_profess_wordlib_has_update = false;
       refresh_update_btn_state();
    });
    m_wordlib_check_thread->start();
    m_profess_wordlib_check_thread->start();

    m_update_wordlib_thread =  QSharedPointer<UpdateSysWordlibThread>(new UpdateSysWordlibThread());
    m_profess_wordlib_update_thread = QSharedPointer<ProfessWordlibDownloadThread>(new ProfessWordlibDownloadThread());
    connect(m_update_wordlib_thread.data(), &UpdateSysWordlibThread::updateFinished, this, &WordlibStackedWidget::slot_upate_wordlib_finished);
    connect(m_profess_wordlib_update_thread.data(),&ProfessWordlibDownloadThread::download_finished,this,[&](int error_count){
        if(error_count == 0){
            m_profess_wordlib_has_update = false;
        }
        else
        {
            m_profess_wordlib_has_update = true;
        }
        refresh_update_btn_state();

    });
    connect(m_profess_wordlib_update_thread.data(),&ProfessWordlibDownloadThread::download_error,this,[&](){
        refresh_update_btn_state();
    });
}

void WordlibStackedWidget::init_wordlib_table_view()
{
    //官方词库列表
    m_official_wordlib_model = wordlib_model(new MyTableModel());
    m_official_wordlib_view = wordlib_view(new MyTableView());
    m_official_wordlib_delegate = wordlib_delegate(new Delegate(m_official_wordlib_view.data()));
    m_official_wordlib_view->setItemDelegate(m_official_wordlib_delegate.data());
    m_official_wordlib_view->setModel(m_official_wordlib_model.data());
    m_official_wordlib_view->verticalHeader()->setDefaultSectionSize(40);
    m_official_wordlib_view->setSelectionBehavior(QAbstractItemView::SelectItems);
    m_official_wordlib_view->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_official_wordlib_view->setProperty("type","wordlib_item_view");
    m_official_wordlib_view->setObjectName("official_table_view");

    //wordlib statechanged and removed
    connect(m_official_wordlib_delegate.data(), &Delegate::wordlib_removed, this, &WordlibStackedWidget::remove_official_wordlib_slot);
    connect(m_official_wordlib_delegate.data(), &Delegate::wordlib_state_changed, this,[&](){slot_update_selected_wordlib();});

    //用户自定义词库列表
    m_user_wordlib_model = wordlib_model(new MyTableModel());
    m_user_wordlib_view = wordlib_view(new MyTableView());
    m_user_wordlib_delegate = wordlib_delegate(new Delegate(m_user_wordlib_view.data()));
    m_user_wordlib_view->setItemDelegate(m_user_wordlib_delegate.data());
    m_user_wordlib_view->setModel(m_user_wordlib_model.data());
    m_user_wordlib_view->verticalHeader()->setDefaultSectionSize(40);
    m_user_wordlib_view->setSelectionBehavior(QAbstractItemView::SelectItems);
    m_user_wordlib_view->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_user_wordlib_view->setProperty("type","wordlib_item_view");
    m_user_wordlib_view->setObjectName("user_table_view");

    //user wordlib remove statechanged and import userwordlib
    connect(m_user_wordlib_delegate.data(), &Delegate::wordlib_removed, this, &WordlibStackedWidget::remove_user_wordlib_slot);
    connect(m_user_wordlib_delegate.data(), &Delegate::wordlib_state_changed, this, [&](){
        slot_update_selected_wordlib();
    });
    connect(m_user_wordlib_delegate.data(), &Delegate::import_user_wordlib_btn_clicked, this, [&](){
        import_category_wordlib();
    });

    //add tableview
    QHBoxLayout* wordlib_tab_layout = new QHBoxLayout(ui->official_wordlib_page);
    wordlib_tab_layout->setMargin(0);
    wordlib_tab_layout->addWidget(m_official_wordlib_view.data());

    QHBoxLayout* user_tab_layout = new QHBoxLayout(ui->user_wordlib_page);
    user_tab_layout->setMargin(0);
    user_tab_layout->addWidget(m_user_wordlib_view.data());
}

bool WordlibStackedWidget::load_default_official_wordlib_list_from_file()
{
    OFFICIAL_DEFAULT_WORDLIB_LIST.clear();
#ifdef Q_OS_LINUX
    QString install_path = TOOLS::PathUtils::GetPackageInstallpath();
    QString wordlib_list_path = QDir::toNativeSeparators(install_path + QString("files/wordlib_list.json"));
#elif defined(Q_OS_WIN)
    QString wordlib_list_path = QApplication::applicationDirPath() + QString("wordlib_list.json");
#endif
    QFile file(wordlib_list_path);
    if(!file.exists())
    {
        return false;
    }
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString value = file.readAll();
    file.close();

    QJsonParseError parseJsonErr;
    QJsonDocument document = QJsonDocument::fromJson(value.toUtf8(),&parseJsonErr);
    if(!(parseJsonErr.error == QJsonParseError::NoError))
    {
        return false;
    }
    QJsonObject jsonObject = document.object();

    QStringList keyList = jsonObject.keys();
    for(int index=0; index<keyList.size(); ++index)
    {

        QString  currentKey = keyList.at(index);
        QJsonObject childObject = jsonObject.value(currentKey).toObject();
        QString wordlib_name = childObject.value("wordlib_name").toString();
        OFFICIAL_DEFAULT_WORDLIB_LIST.append(WordlibListInfo(currentKey,wordlib_name));
    }
    return true;
}


void WordlibStackedWidget::load_wordlib()
{
    load_official_wordlib();
    load_custom_wordlib();
    //添加占位的词库信息,用来在用户词库页签里面导入词库
    QSharedPointer<TAB_WORDLIB_INFO> add_wordlib_func = QSharedPointer<TAB_WORDLIB_INFO>(new TAB_WORDLIB_INFO);
    m_user_wordlib_info_vector.push_back(add_wordlib_func);

    //添加本地没有的文件
    m_official_wordlib_model->set_data(m_offical_wordlib_info_vector);
    m_user_wordlib_model->set_data(m_user_wordlib_info_vector);
    slot_update_custom_wordlib_list();
}

void WordlibStackedWidget::load_official_wordlib()
{
    //获取默认的官方词库列表
    m_offical_wordlib_info_vector.clear();
    load_default_official_wordlib_list_from_file();
    m_officail_wordlib_list = get_official_wordlib_list();

    //get selected wordlib list from config
#ifdef Q_OS_WIN
#elif defined(Q_OS_LINUX)
    QString wordlib_list;
    ConfigManager::Instance()->GetStrConfigItem("selected_wordlibs",wordlib_list);
    QStringList names = wordlib_list.split(",");
    QVector<QString> selected_wordlib_vector = names.toVector();
#endif
    //parser the wordlib info
    for(WordlibListInfo index_info : m_officail_wordlib_list)
    {
        QString file_name = index_info.fileName;
        QString url = index_info.download_url;
        QString view_name = index_info.viewName;

        //过滤掉中间产生的V3V5词库
        QString file_path = TOOLS::PathUtils::get_install_wordlib_dir_path() + file_name;
        QString local_offical_path = TOOLS::PathUtils::get_cloud_wordlib_dir() + file_name;
        QFileInfo file_info(file_path);
        QFileInfo local_file_info(local_offical_path);
        if (file_info.exists() && file_info.isFile()  && Utils::IsValidWordLibFile(file_info))
        {
            WORDLIB_INFO wordlib_info;
            if(TOOLS::WordlibUtils::get_uwl_fileinfo(file_path, wordlib_info))
            {
                 QSharedPointer<TAB_WORDLIB_INFO> index_info = QSharedPointer<TAB_WORDLIB_INFO>(new TAB_WORDLIB_INFO);
                 index_info->wordlib_name = wordlib_info.wordlib_name.trimmed();
                 index_info->wordlib_item_count = wordlib_info.words_count;
                 index_info->current_state = selected_wordlib_vector.contains(file_info.fileName())? CHECKED:UN_CHECKED;
                 index_info->autor_name = wordlib_info.author;
                 index_info->update_time = wordlib_info.wordlib_last_modified_date;
                 index_info->file_path = file_info.filePath();
                 index_info->file_name = file_info.fileName();
                 index_info->can_be_edit = wordlib_info.can_be_edit;
                 index_info->is_official_wordlib = true;
                 m_offical_wordlib_info_vector.push_back(index_info);
            }
        }
        else if(local_file_info.exists() && local_file_info.isFile()  && Utils::IsValidWordLibFile(local_file_info))
        {
            WORDLIB_INFO wordlib_info;
            if(TOOLS::WordlibUtils::get_uwl_fileinfo(local_offical_path, wordlib_info))
            {
                 QSharedPointer<TAB_WORDLIB_INFO> index_info = QSharedPointer<TAB_WORDLIB_INFO>(new TAB_WORDLIB_INFO);
                 index_info->wordlib_name = wordlib_info.wordlib_name.trimmed();
                 index_info->wordlib_item_count = wordlib_info.words_count;
                 index_info->current_state = selected_wordlib_vector.contains(file_info.fileName())? CHECKED:UN_CHECKED;
                 index_info->autor_name = wordlib_info.author;
                 index_info->update_time = wordlib_info.wordlib_last_modified_date;
                 index_info->file_path = local_file_info.filePath();
                 index_info->file_name = local_file_info.fileName();
                 index_info->can_be_edit = wordlib_info.can_be_edit;
                 index_info->is_official_wordlib = true;
                 m_offical_wordlib_info_vector.push_back(index_info);
            }
        }
        else if(!file_info.exists())
        {
            QSharedPointer<TAB_WORDLIB_INFO> index_info = QSharedPointer<TAB_WORDLIB_INFO>(new TAB_WORDLIB_INFO);
            index_info->current_state = CLOUD_WORDLIB;
            index_info->wordlib_name = view_name;
            index_info->download_url = url;
            index_info->file_name = file_name;
            m_offical_wordlib_info_vector.push_back(index_info);
        }

    }
}

void WordlibStackedWidget::load_custom_wordlib()
{
    m_user_wordlib_info_vector.clear();
    m_custom_wordlib_list.clear();

#ifdef Q_OS_WIN
    config_info temp_config;
    if(Config::Instance()->GetConfigItemByJson("custom_wordlib_list",temp_config))
    {
        m_custom_wordlib_list = temp_config.itemCurrentStrValue.split(";");
    }
#elif defined(Q_OS_LINUX)
    m_custom_wordlib_list;
    QString customer_wordlib_str;
    ConfigManager::Instance()->GetStrConfigItem("custom_wordlib_list",customer_wordlib_str);
    m_custom_wordlib_list = customer_wordlib_str.split(";");
    QString wordlib_list;
    ConfigManager::Instance()->GetStrConfigItem("selected_wordlibs",wordlib_list);
    QStringList names = wordlib_list.split(",");
    QVector<QString> selected_wordlib_vector = names.toVector();
#endif

    for(QString index_wordlib_name : m_custom_wordlib_list)
    {
        //过滤掉中间产生的V3V5词库
        if(index_wordlib_name.isEmpty()){
            continue;
        }
        QString file_path = TOOLS::PathUtils::get_cloud_wordlib_dir() + index_wordlib_name;
        QFileInfo file_info(file_path);
        if (file_info.exists() && file_info.isFile()  && Utils::IsValidWordLibFile(file_info))
        {
            WORDLIB_INFO wordlib_info;
            if(TOOLS::WordlibUtils::get_uwl_fileinfo(file_path, wordlib_info))
            {
                 QSharedPointer<TAB_WORDLIB_INFO> index_info = QSharedPointer<TAB_WORDLIB_INFO>(new TAB_WORDLIB_INFO);
                 index_info->wordlib_name = wordlib_info.wordlib_name.trimmed();
                 index_info->wordlib_item_count = wordlib_info.words_count;
                 index_info->current_state = selected_wordlib_vector.contains(file_info.fileName())? CHECKED:UN_CHECKED;
                 index_info->autor_name = wordlib_info.author;
                 index_info->update_time = wordlib_info.wordlib_last_modified_date;
                 index_info->file_path = file_info.filePath();
                 index_info->file_name = file_info.fileName();
                 index_info->can_be_edit = wordlib_info.can_be_edit;
                 index_info->is_official_wordlib = false;
                 m_user_wordlib_info_vector.push_back(index_info);
            }
        }
    }
}



bool WordlibStackedWidget::add_wordlib_to_container(QString file_path, bool is_official)
{
    if(!QFile(file_path).exists() || (QFileInfo(file_path).suffix() != QString("uwl")))
    {
        return false;
    }

    QFileInfo file_info(file_path);
    if(SYS_WORLIDB_LIST.contains(file_info.fileName()))
        return false;
    WORDLIB_INFO wordlib_info;
    if (file_info.isFile()  && Utils::IsValidWordLibFile(file_info))
    {
        if(TOOLS::WordlibUtils::get_uwl_fileinfo(file_info.filePath(), wordlib_info))
        {
             QSharedPointer<TAB_WORDLIB_INFO> index_info = QSharedPointer<TAB_WORDLIB_INFO>(new TAB_WORDLIB_INFO);
             index_info->wordlib_name = wordlib_info.wordlib_name.trimmed();
             index_info->wordlib_item_count = wordlib_info.words_count;
             index_info->current_state = CHECKED;
             index_info->autor_name = wordlib_info.author;
             index_info->update_time = wordlib_info.wordlib_last_modified_date;
             index_info->file_path = file_path;
             index_info->file_name = file_info.fileName();
             index_info->can_be_edit = wordlib_info.can_be_edit;
             index_info->is_official_wordlib = is_official;
             if(is_official)
             {
                for(int index=0; index<m_offical_wordlib_info_vector.size(); ++index)
                {
                    if(m_offical_wordlib_info_vector.at(index)->file_name == index_info->file_name)
                    {
                        m_offical_wordlib_info_vector[index] = index_info;
                        break;
                    }
                }
             }
             else
             {
                int current_size = m_user_wordlib_info_vector.size();
                m_user_wordlib_info_vector.insert(current_size-1,index_info);
                m_custom_wordlib_list.push_back(file_info.fileName());
             }

             is_official?refresh_wordlib_table_view(OFFICIAL_ONLY):refresh_wordlib_table_view(CUSTOM_ONLY);

             if(!is_official){
                 slot_update_custom_wordlib_list();
             }
             slot_update_selected_wordlib();
             return true;

        }

    }
    return false;
}

void WordlibStackedWidget::refresh_wordlib_table_view(UpdateType update_type)
{
    if(update_type == BOTH_UPDATE)
    {
        m_official_wordlib_model->set_data(m_offical_wordlib_info_vector);
        m_user_wordlib_model->set_data(m_user_wordlib_info_vector);
    }
    else if(update_type == OFFICIAL_ONLY)
    {
        m_official_wordlib_model->set_data(m_offical_wordlib_info_vector);
    }
    else if(update_type == CUSTOM_ONLY)
    {
        m_user_wordlib_model->set_data(m_user_wordlib_info_vector);
        
    }
}

QString WordlibStackedWidget::get_output_file_path()
{

    QFileDialog *fileDialog = new QFileDialog(this);
    fileDialog->setWindowTitle("保存用户词库");
    fileDialog->setAcceptMode(QFileDialog::AcceptSave);
    fileDialog->setFileMode(QFileDialog::AnyFile);
    fileDialog->setViewMode(QFileDialog::Detail);

    fileDialog->setDirectory(".");
    fileDialog->setNameFilters(QStringList() << "File (*.txt)" << "uwl wordlib(*.uwl)");
    fileDialog->selectFile(QString("user.%1").arg("txt"));

    QTimer * timer = new QTimer(this);
    timer->setInterval(300);

    QString prev_type = fileDialog->selectedNameFilter();

    connect(timer,&QTimer::timeout,this,[&](){
        QString type_filter = fileDialog->selectedNameFilter();
        if(type_filter != prev_type)
        {
            prev_type = type_filter;
            QString file_path = fileDialog->selectedFiles().at(0);
            QFileInfo file_info(file_path);
            QString base_name = file_info.baseName();
            if(type_filter == "File (*.txt)")
            {
                fileDialog->selectFile(QString("%1.%2").arg(base_name).arg("txt"));
            }
            else
            {
                fileDialog->selectFile(QString("%1.%2").arg(base_name).arg("uwl"));
            }
        }
    });
    timer->start();

    if(fileDialog->exec() == QDialog::Accepted)
    {
        timer->stop();
        QString path = fileDialog->selectedFiles().at(0);
        return path;
    }
    timer->stop();
    return QString("");
}

void WordlibStackedWidget::slot_on_btn_clicked()
{
    QObject *object = QObject::sender();
    QPushButton *push_button = qobject_cast<QPushButton *>(object);
    if(push_button == ui->user_wordlib_import_btn)
    {
        slot_import_user_wordlib();
    }
    else if(push_button == ui->user_wordlib_export_btn)
    {
        export_user_wordlib();
    }
    else if(push_button == ui->clear_user_wordlib_btn)
    {
        clear_user_wordlib();
    }
    else if(push_button == ui->update_category_wordlib_btn)
    {
        update_category_wordlib();
    }
    else if(push_button == ui->export_category_wordlib_btn)
    {
        export_category_wordlib();
    }
    else if(push_button == ui->import_category_wordlib_btn)
    {
        import_category_wordlib();
    }

}
void WordlibStackedWidget::ResetConfigInfo()
{
    QStringList config_item_name_list;
    config_item_name_list << "local_wordlib_network_update"
                          << "auto_update"
                          << "selected_wordlibs";


    for(QString indexKey: config_item_name_list)
    {
        ConfigManager::Instance()->ClearConfigItem(indexKey);
    }
    LoadConfigInfo();
}

QVector<WordlibListInfo> WordlibStackedWidget::get_official_wordlib_list()
{
    QNetworkConfigurationManager networkConfig;
    if(!networkConfig.isOnline())
    {
        return OFFICIAL_DEFAULT_WORDLIB_LIST;
    }

    m_cloud_official_wordlib_list.clear();
    int returnCode = NetworkHandler::Instance()->get_profess_wordlib_list(m_cloud_official_wordlib_list);
    if(returnCode != 0)
    {
        return OFFICIAL_DEFAULT_WORDLIB_LIST;
    }
    else
    {
        //两个名单组合到一起使用
        auto check_exist = [=](WordlibListInfo indexInfo)->bool{
            int size =OFFICIAL_DEFAULT_WORDLIB_LIST.size();
            for(int index=0; index<size; ++index){
                if(OFFICIAL_DEFAULT_WORDLIB_LIST.at(index).fileName == indexInfo.fileName)
                {
                    return true;
                }
            }
            return false;
        };
        QVector<WordlibListInfo> result_wordlib_info_vector;
        result_wordlib_info_vector = OFFICIAL_DEFAULT_WORDLIB_LIST;
        int cloud_size = m_cloud_official_wordlib_list.size();
        for(int official_index=0; official_index<cloud_size; ++official_index){
            WordlibListInfo current_list_info = m_cloud_official_wordlib_list.at(official_index);
            if(!check_exist(current_list_info))
            {
               result_wordlib_info_vector.push_back(current_list_info);
            }
        }

        return result_wordlib_info_vector;
    }

}

void WordlibStackedWidget::slot_upate_wordlib_finished(int state)
{
    if(state == 0)
    {
        m_sys_wordlib_has_update = false;

        //系统词库同步成功之后，通知引擎加载

        emit wordlibChanged();
    }
    else if(state == 9)
    {
        m_sys_wordlib_has_update = true;
    }
    else
    {
        m_sys_wordlib_has_update = true;
    }

    refresh_update_btn_state();
    slot_notify_wordlib_update_result();

}

void WordlibStackedWidget::slot_update_selected_wordlib()
{
    QVector<QString> wordlib_container;
    auto insert_system_default = [&](QString file_name){
        wordlib_container.push_back(file_name);
    };
    std::for_each(SYS_WORLIDB_LIST.cbegin(),SYS_WORLIDB_LIST.cend(),insert_system_default);

    auto refresh_selected_wordlib = [&](const QSharedPointer<TAB_WORDLIB_INFO>& tab_info){

        if(tab_info->current_state == CHECKED){
            wordlib_container.push_back(tab_info->file_name);
        }
    };

    std::for_each(m_offical_wordlib_info_vector.cbegin(),m_offical_wordlib_info_vector.cend(),refresh_selected_wordlib);
    std::for_each(m_user_wordlib_info_vector.cbegin(),m_user_wordlib_info_vector.cend(),refresh_selected_wordlib);

#ifdef Q_OS_LINUX
    QStringList wordlib_name_list = wordlib_container.toList();
    ConfigManager::Instance()->SetStrConfigItem("selected_wordlibs",wordlib_name_list.join(","));
#endif
}

void WordlibStackedWidget::slot_update_custom_wordlib_list()
{
    QString custom_result_str;
    auto update_user_wordlib_list = [&](QSharedPointer<TAB_WORDLIB_INFO> tab_info){
        if(tab_info->file_name.isEmpty())
        {
            return;
        }
        custom_result_str.append(tab_info->file_name);
        custom_result_str.append(";");
    };
    std::for_each(m_user_wordlib_info_vector.cbegin(),m_user_wordlib_info_vector.cend(),update_user_wordlib_list);
#ifdef Q_OS_WIN
    config_info custom_info;
    if(!custom_result_str.isEmpty()){
        custom_info.itemName = "custom_wordlib_list";
        custom_info.itemCurrentStrValue = custom_result_str;
        custom_info.itemType = "string";
        //Config::Instance()->SetConfigItemByJson("custom_wordlib_list",custom_info);
    }
#elif defined(Q_OS_LINUX)
    ConfigManager::Instance()->SetStrConfigItem("custom_wordlib_list",custom_result_str);
#endif
}

void WordlibStackedWidget::slot_notify_wordlib_update_result()
{

}

void WordlibStackedWidget::slot_import_user_wordlib()
{
    QString input_file_path = QFileDialog::getOpenFileName(NULL, tr("打开需要导入的词库"),
                                                        "",
                                                        tr("wordlib format(*.txt *.uwl)"));
    if(input_file_path.isEmpty())
    {
        return;
    }

    QFileInfo wordlib_info(input_file_path);

    auto EnableUserWordlib = [&]()
    {
    #ifdef Q_OS_LINUX
                ConfigBus::instance()->valueChanged("loadWordlib", "");                
    #endif
    };

    ConfigBus::instance()->valueChanged("saveUserWordlib", "");
    ConfigBus::instance()->valueChanged("userWordlibCanSave", 0);

    if(wordlib_info.suffix() == "txt")
    {
        input_file_path = transform_codec(input_file_path);
        QStringList param;
        param << "-combine-uwl-text";
        param << input_file_path;
        param << ConfigManager::Instance()->GetUserUwlPath();
        qDebug() << param;
        QString process_name(TOOLS::PathUtils::get_wordlibtool_path());

        QString user_uwl_file_path = ConfigManager::Instance()->GetUserUwlPath();
        QFile user_uwl_file(user_uwl_file_path);

        if(!user_uwl_file.exists())
        {
            ConfigBus::instance()->valueChanged("loadWordlib", "");
        }

        if(!user_uwl_file.exists())
        {
            QString user_uwl_path = ConfigManager::Instance()->GetUserUwlPath();
            if (!TOOLS::WordlibUtils::create_uwl_wordlib_from_text(input_file_path, user_uwl_path))
            {
                TOOLS::Messgebox::notice_msg(QString("导入失败"));
                return;
            }
            else
            {
                TOOLS::LogUtils::write_log_to_file("import user wordlib succeed");
                TOOLS::Messgebox::notice_msg("导入成功");
                return;
            }
        }

        if (!QFileInfo(process_name).exists())
        {
            TOOLS::Messgebox::notice_msg(QString::fromLocal8Bit("找不到导库工具, 导入失败！"));
            return;
        }
        QProcess process;
        int ret = process.execute(process_name, param);
        if(ret >= 0)
        {
            TOOLS::LogUtils::write_log_to_file("import user wordlib succeed");           
            TOOLS::Messgebox::notice_msg("导入成功");
        }
        else
        {
            TOOLS::LogUtils::write_log_to_file("when combine user uwl throw error failed");            
            TOOLS::Messgebox::notice_msg("导入失败");

        }
        EnableUserWordlib();
    }
    else if (wordlib_info.suffix() == "uwl")
    {
        WORDLIB_INFO input_wordlib_info;
        if(TOOLS::WordlibUtils::get_uwl_fileinfo(input_file_path,input_wordlib_info) && !input_wordlib_info.can_be_edit)
        {
            TOOLS::LogUtils::write_log_to_file("import user wordlib is invalid");
            TOOLS::Messgebox::notice_msg("非可编辑词库\n不可导入");
            EnableUserWordlib();
            return;
        }
        QString user_wordlib_path = TOOLS::PathUtils::get_user_wordlib_path();
        if(!QFile::exists(user_wordlib_path))
        {
            QFile::copy(input_file_path,user_wordlib_path);            
            TOOLS::LogUtils::write_log_to_file("import user wordlib succeed");
            TOOLS::Messgebox::notice_msg("导入成功");
            EnableUserWordlib();
            return;
        }
        else
        {
            if(TOOLS::WordlibUtils::merge_uwl_wordlib(user_wordlib_path,input_file_path))
            {                
                TOOLS::LogUtils::write_log_to_file("import user wordlib succeed");
                TOOLS::Messgebox::notice_msg("导入成功");
            }
            else
            {
                TOOLS::LogUtils::write_log_to_file("when combine user uwl throw error failed");
                TOOLS::Messgebox::notice_msg("导入失败");

            }
            EnableUserWordlib();
        }
    }
}



void WordlibStackedWidget::refresh_update_btn_state()
{
    //由于和后台服务更新的管理员权限词库冲突/暂时隐藏更新按钮
    if(m_sys_wordlib_has_update || m_profess_wordlib_has_update)
    {
        ui->update_category_wordlib_btn->setVisible(false);
    }
    else
    {
         ui->update_category_wordlib_btn->setVisible(false);
    }
}

QString WordlibStackedWidget::transform_codec(QString file_path)
{
    if(!QFile::exists(file_path))
    {
        return "";
    }
    QFile input_file(file_path);
    input_file.open(QFile::ReadOnly);

    QTextStream  read_stream(&input_file);
    if(Utils::GetFileCodeType(file_path.toStdString().c_str()) == UTF16LE)
    {
        QTextCodec *codec = QTextCodec::codecForName("UTF-16LE");
        read_stream.setCodec(codec);
        QString file_content = read_stream.readAll();
        input_file.close();

        QString tmp_path = "/tmp/tmpinput.txt";
        if(QFile::exists(tmp_path))
        {
            QFile::remove(tmp_path);
        }
        QFile output_file(tmp_path);
        output_file.open(QFile::ReadWrite);
        QTextStream outputStream(&output_file);
        outputStream.setCodec("UTF-8");
        outputStream << file_content;
        output_file.close();
        return tmp_path;
    }
    else
    {
        return file_path;
    }
}


void ProfessWordlibDownloadThread::run()
{
    try {
        QVector<WordlibListInfo> custom_wordlib_vector;
        QVector<WordlibListInfo> has_update_info_vector;
        has_update_info_vector.clear();
        custom_wordlib_vector.clear();
        int handle_code = NetworkHandler::Instance()->get_profess_wordlib_list(custom_wordlib_vector);
        Q_UNUSED(handle_code);
        int error_wordlib_count = 0;
        auto check_md5 = [&](WordlibListInfo index_info){
            QString file_path;//= Config::wordLibDirPath() + index_info.fileName;
            if(QFile(file_path).exists())
            {
                QString file_md5 = Utils::GetFileMD5(file_path);
                if(file_md5 != index_info.md5)
                {
                    int returnCode = NetworkHandler::Instance()->DownloadFileFromWeb(index_info.download_url,/*Config::Instance()->wordLibDirPath()*/"",
                                                                                     file_path,index_info.fileName);
                    QString new_md5 = Utils::GetFileMD5(file_path);
                    if(returnCode != 0){
                        emit download_error(index_info.viewName);
                        ++error_wordlib_count;
                    }
                }
            }

        };
        std::for_each(custom_wordlib_vector.cbegin(),custom_wordlib_vector.cend(),check_md5);
        emit download_finished(error_wordlib_count);

    } catch (std::exception& e) {
        Utils::WriteLogToFile(QString("download custom wordlib throw exception:%1").arg(e.what()));
    }
}

void ProfessWordlibCheckThread::run()
{
    try
    {
        QVector<WordlibListInfo> custom_wordlib_vector;
        QVector<WordlibListInfo> has_update_info_vector;
        has_update_info_vector.clear();
        custom_wordlib_vector.clear();
        int handle_code = NetworkHandler::Instance()->get_profess_wordlib_list(custom_wordlib_vector);
        Q_UNUSED(handle_code);
        auto check_md5 = [&](WordlibListInfo index_info){
            QString file_path ;//= Config::wordLibDirPath() + index_info.fileName;
            if(QFile(file_path).exists())
            {
                QString file_md5 = Utils::GetFileMD5(file_path);
                WORDLIB_INFO index_wordlib_info;
                TOOLS::WordlibUtils::get_uwl_fileinfo(file_path,index_wordlib_info);
                if((file_md5 != index_info.md5)&&(index_info.viewName.compare(index_wordlib_info.wordlib_name) == 0))
                {
                    has_update_info_vector.push_back(index_info);
                }
            }

        };
        std::for_each(custom_wordlib_vector.cbegin(),custom_wordlib_vector.cend(),check_md5);
        if(has_update_info_vector.size()>0)
        {
            emit profess_wordlib_has_update();
        }
        else
        {
            emit profess_wordlib_has_no_update();
        }
    }
    catch (std::exception& e)
    {
        Utils::WriteLogToFile(QString("profess check throw exception:%1").arg(e.what()));
    }

}

void WordlibStackedWidget::export_user_wordlib()
{
    QString user_wordlib_path = TOOLS::PathUtils::get_user_wordlib_path();
    if(!QFile::exists(user_wordlib_path))
    {
        TOOLS::Messgebox::notice_msg("用户词库不存在");
        return;
    }
   QString output_path = get_output_file_path();

   if(output_path.isEmpty())
   {
        return;
   }

   QFileInfo fileInfo(output_path);
   if(fileInfo.suffix().compare("uwl") == 0)
   {
       QFile::copy(user_wordlib_path,output_path);
       TOOLS::Messgebox::notice_msg("导出成功");
       return;
   }
   bool flag = TOOLS::WordlibUtils::export_uwl_wordlib_to_text(user_wordlib_path,output_path);
   if(flag)
   {
       TOOLS::Messgebox::notice_msg("导出成功");
   }
   else
   {
       TOOLS::Messgebox::notice_msg("导出失败");
   }
}

void WordlibStackedWidget::clear_user_wordlib()
{
    if(TOOLS::Messgebox::notice_msg("确认清空\n用户词库",0,1) == QDialog::Accepted)
    {
        QString user_wordlib_path = TOOLS::PathUtils::get_user_wordlib_path();
        ConfigBus::instance()->valueChanged("clearUserWordlib", "");
        if(QFile::remove(user_wordlib_path))
        {
            ConfigBus::instance()->valueChanged("loadWordlib", "");
            TOOLS::Messgebox::notice_msg("用户词库清空成功");
            return;
        }
        else
        {
            TOOLS::Messgebox::notice_msg("用户词库不存在");
        }
    }
}

void WordlibStackedWidget::update_category_wordlib()
{
    ui->update_category_wordlib_btn->setVisible(false);
    if(m_sys_wordlib_has_update){
        m_update_wordlib_thread->start();
    }
    if(m_profess_wordlib_has_update){
        m_profess_wordlib_update_thread->start();
    }
}

void WordlibStackedWidget::export_category_wordlib()
{
    int selected_num = 0;
    QVector<QString>  can_not_export_wordlib_vector;
    QVector<QString>  can_export_wordlib_vector;
    can_not_export_wordlib_vector.clear();
    can_export_wordlib_vector.clear();

    //查找用户选中的需要导出的词库
    auto count_table_select = [&](wordlib_info_ptr info_ptr){
        if(info_ptr->is_item_selected_in_ui)
        {
            selected_num++;
            if(!info_ptr->can_be_edit)
            {
                can_not_export_wordlib_vector.push_back(info_ptr->wordlib_name);
            }
            can_export_wordlib_vector.push_back(info_ptr->wordlib_name);
        }

    };
    if(ui->stackedWidget->currentIndex() == 0)
    {
        std::for_each(m_offical_wordlib_info_vector.begin(),m_offical_wordlib_info_vector.end(),count_table_select);
    }
    else if(ui->stackedWidget->currentIndex() == 1)
    {
        std::for_each(m_user_wordlib_info_vector.begin(),m_user_wordlib_info_vector.end(),count_table_select);
    }

    if(selected_num == 0)
    {
        TOOLS::Messgebox::notice_msg("没有可导出的词库");
        return;
    }

    m_outputWordlibDlg->SetWordlibVector(can_export_wordlib_vector);
    if(m_outputWordlibDlg->exec() == QDialog::Rejected)
    {
        return;
    }

    can_export_wordlib_vector.clear();
    can_export_wordlib_vector = m_outputWordlibDlg->GetWordlibNameVector();
    if(can_export_wordlib_vector.size() == 0)
    {
        TOOLS::Messgebox::notice_msg("没有可导出的词库");
        return;
    }

    QString selectedFilePath = m_outputWordlibDlg->GetSaveDirPath();
    QString output_type = m_outputWordlibDlg->GetOutputFileType();

    auto output_wordlib_file = [&](QString wordlib_name)
    {
        QString uwl_path = find_wordlibpath_by_name(wordlib_name);
        if(uwl_path.isEmpty()){return;}

        QFileInfo info(uwl_path);
        QString targetTextPath = QDir::toNativeSeparators(selectedFilePath + "/" + info.baseName() + ".txt");
        bool ret = TOOLS::WordlibUtils::export_uwl_wordlib_to_text(uwl_path,targetTextPath);
        if(!ret)
        {
            TOOLS::Messgebox::notice_msg(QString("%1\n导出失败").arg(wordlib_name));
        }
    };

    //export uwl file
    auto output_uwl_file = [&](QString wordlib_name){

        QString uwl_path = find_wordlibpath_by_name(wordlib_name);
        if(uwl_path.isEmpty()){return;}
        QFileInfo info(uwl_path);
        QString targetPath = QDir::toNativeSeparators(selectedFilePath + "/" + info.baseName() + ".uwl");
        TOOLS::WordlibUtils::export_uwl_wordlib_to_uwl(uwl_path,targetPath);
    };

    if(output_type.compare("词库文件(.uwl)") == 0)
    {
        std::for_each(can_export_wordlib_vector.begin(),can_export_wordlib_vector.end(),output_uwl_file);
    }
    else if(output_type.compare("文本文件(.txt)") == 0)
    {
        QString file_name_str;
        if(can_not_export_wordlib_vector.size()>0)
        {
            auto combine_str = [&](QString file_name){
                file_name_str.append(file_name);
                file_name_str.append(" ");
            };
            std::for_each(can_not_export_wordlib_vector.begin(),can_not_export_wordlib_vector.end(),combine_str);
            TOOLS::Messgebox::notice_msg(QString("%1\n无法导出文本").arg(file_name_str));
            return;
        }
        std::for_each(can_export_wordlib_vector.begin(),can_export_wordlib_vector.end(),output_wordlib_file);
    }
    TOOLS::Messgebox::notice_msg("导出完成");
}

void WordlibStackedWidget::import_category_wordlib()
{
    QString input_file_path = QFileDialog::getOpenFileName(this, tr("打开需要导入的词库"),
                                                           "",
                                                           tr("wordlib format (*.uwl *.txt)"));
    if(input_file_path.isEmpty())
    {
        return;
    }

    //判断是否是官方词库
    QString target_file_name = QFileInfo(input_file_path).baseName() + ".uwl";
    auto find_pos = std::find_if(m_officail_wordlib_list.cbegin(),m_officail_wordlib_list.cend(),[&](WordlibListInfo fileinfo){
            return fileinfo.fileName == target_file_name;});

    bool is_official_wordlib = false;
    if(find_pos != m_officail_wordlib_list.end())
    {
        is_official_wordlib = true;
    }
    if(is_official_wordlib)
    {
        TOOLS::Messgebox::notice_msg("文件名称与官方词库冲突");
        return;
    }
    QStringList system_wordlib_list;
    system_wordlib_list << "sys.uwl" << "user.uwl";
    if(system_wordlib_list.contains(target_file_name))
    {
        TOOLS::Messgebox::notice_msg("文件名称与官方词库冲突");
        return;
    }
    QString target_file_path = TOOLS::PathUtils::get_cloud_wordlib_dir() + QFileInfo(input_file_path).baseName() + ".uwl";
    if(QFile::exists(target_file_path))
    {
        TOOLS::Messgebox::notice_msg("文件名称与已有文件冲突");
        return;
    }

    QString wordlib_type_str = is_official_wordlib? QString("官方词库"):QString("自定义词库");

    QFileInfo wordlib_info(input_file_path);
    if(wordlib_info.suffix() == "txt")
    {
        QString tmp_dir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
        QString import_uwl_file_path = QString("%1/%2.uwl")
                .arg(tmp_dir)
                .arg(wordlib_info.baseName());

        input_file_path = transform_codec(input_file_path);

        if (!TOOLS::WordlibUtils::create_uwl_wordlib_from_text(input_file_path, import_uwl_file_path))
        {
            TOOLS::LogUtils::write_log_to_file(QString("when transform txt to uwl error"));
            TOOLS::Messgebox::notice_msg(QString("导入%1\n失败").arg(wordlib_type_str));
            return;
        }

        if(!QFile::copy(import_uwl_file_path,target_file_path))
        {
            TOOLS::Messgebox::notice_msg(QString("导入%1\n失败").arg(wordlib_type_str));
            return;
        }


        if(add_wordlib_to_container(target_file_path,is_official_wordlib))
        {
            QFile(import_uwl_file_path).remove();
            (is_official_wordlib)?ui->official_wordlib_tab_btn->click() : ui->user_defined_wrodlib_tab_btn->click();
            TOOLS::LogUtils::write_log_to_file("import user wordlib succeed");
            TOOLS::Messgebox::notice_msg(QString("导入%1\n成功").arg(wordlib_type_str));
        }
        else
        {
            TOOLS::LogUtils::write_log_to_file("when combine user uwl throw error failed");
            QFile(import_uwl_file_path).remove();
            TOOLS::Messgebox::notice_msg(QString("导入%1\n失败").arg(wordlib_type_str));
        }
    }
    else if ((wordlib_info.suffix() == "uwl"))
    {
        if(QFile::copy(input_file_path,target_file_path)&&(add_wordlib_to_container(target_file_path,is_official_wordlib)))
        {

            (is_official_wordlib)?ui->official_wordlib_tab_btn->click() : ui->user_defined_wrodlib_tab_btn->click();
            TOOLS::LogUtils::write_log_to_file("import user wordlib succeed");
            TOOLS::Messgebox::notice_msg(QString("导入%1\n成功").arg(wordlib_type_str));
        }
        else
        {
            TOOLS::LogUtils::write_log_to_file("when combine user uwl throw error failed");
            TOOLS::Messgebox::notice_msg(QString("导入%1\n失败").arg(wordlib_type_str));
        }
    }

}
