#include "wordlibstackedwidget.h"
#include "ui_wordlibstackedwidget.h"
#include "wordlibapi.h"
#include "customize_ui/customizebtngroup.h"
#include "informmsgbox.h"

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
#include "networkhandler.h"
#include <QDateTime>
#include <QTextStream>
#include <QTextCodec>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include "config.h"
#include "dpiadaptor.h"
#include "outputwordlibdlg.h"
#include "mainwindow.h"
#include "WLConverter.h"

#include <memory>
#include <QNetworkConfigurationManager>

const QStringList WordlibStackedWidget::SYS_WORLIDB_LIST = QStringList() << "sys.uwl" << "user.uwl" << "syscloud.uwl";

bool WordlibStackedWidget::AnalysisUwl(const QString& uwl_file_path, WORDLIB_INFO& wordlib_info)
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
    QString wordlib_name = QString::fromWCharArray(wl->header.name);

    wordlib_info.file_path = uwl_file_path;
    wordlib_info.file_name = file_info.fileName();
    wordlib_info.author = QString::fromWCharArray(wl->header.author_name);

    wordlib_info.wordlib_name = wordlib_name;
    wordlib_info.words_count = wl->header.word_count;
    wordlib_info.can_be_edit = wl->header.can_be_edit;

    wordlib_info.md5 = Utils::GetFileMD5(wordlib_info.file_path);
    wordlib_info.wordlib_last_modified_date = file_info.lastModified().toString("yyyy/MM/dd");
    delete buffer;

    return true;
}

bool WordlibStackedWidget::combine_wordlib(QString dest_wordlib, QString wordlib_to_combine)
try
{
    QString install_dir = Config::Instance()->GetInstallDir();
    QString wordlib_tools_path;
    if(install_dir.isEmpty())
    {
        return false;
    }
    else
    {
        wordlib_tools_path = install_dir + "\\WordlibTool.exe";
    }
    if(!QFile::exists(wordlib_tools_path))
    {
        return false;
    }
    QStringList argumentList;
    argumentList << dest_wordlib << wordlib_to_combine;

    QProcess process;
    process.setProgram(wordlib_tools_path);
    process.setArguments(argumentList);
    process.start();
    return process.waitForFinished(5000);
}
catch(std::exception& e)
{
    Utils::WriteLogToFile(QString("when combine wordlib throw exception:%1").arg(e.what()));
    return false;
}



WordlibStackedWidget::WordlibStackedWidget(QWidget *parent) :
    BaseStackedWidget(parent),
    ui(new Ui::WordlibStackedWidget)
{
    ui->setupUi(this);
    init_widget();
    init_function_map();
    init_wordlib_thread();
    //LoadConfigInfo();
}

WordlibStackedWidget::~WordlibStackedWidget()
{
    //退出并销毁对应的线程
    delete ui;
}

void WordlibStackedWidget::ReloadWordlibInfo()
{

}

void WordlibStackedWidget::LoadConfigInfo()
{
    load_wordlib();
    ui->auto_upate_checkbox->disconnect();
    ConfigItemStruct configInfo;
    if(Config::Instance()->GetConfigItemByJson("local_wordlib_network_update",configInfo))
    {
        ui->auto_upate_checkbox->setChecked(configInfo.itemCurrentIntValue);
    }
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
    ConfigItemStruct configInfo;
    configInfo.itemName = "local_wordlib_network_update";
    configInfo.itemCurrentIntValue = (state == Qt::Checked)? 1:0;
    Config::Instance()->SetConfigItemByJson("local_wordlib_network_update",configInfo);

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
    for_each(m_offical_wordlib_info_vector.begin(),m_offical_wordlib_info_vector.end(),remove_official_path);
    if(is_deleted){
        refresh_wordlib_table_view(OFFICIAL_ONLY);
        slot_update_selected_wordlib();
    }
    else
    {
        Utils::NoticeMsgBox("词库无法删除",this);
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
    for_each(m_user_wordlib_info_vector.begin(),m_user_wordlib_info_vector.end(),remove_user_path);
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
    for_each(all_btn.begin(), all_btn.end(),set_btn_style);

    QList<QLabel*> h1_style_list;
    h1_style_list << ui->category_wordlib_label << ui->wordlib_set_label;
    auto set_h1_label = [this](QLabel* label){
        label->setProperty("type","h1");
    };
    for_each(h1_style_list.begin(),h1_style_list.end(),set_h1_label);
    ui->chinese_wordlib_label->setProperty("type","h2");

    QList<QPushButton*> wordlib_tab_list;
    wordlib_tab_list << ui->user_defined_wrodlib_tab_btn << ui->official_wordlib_tab_btn;
    auto set_tab_style = [this](QPushButton* btn){
        btn->setProperty("type","wordlib_tab");
        btn->setCheckable(true);
        btn->setCursor(Qt::PointingHandCursor);
        m_wordlib_tab_group.addButton(btn);
    };
    for_each(wordlib_tab_list.begin(),wordlib_tab_list.end(),set_tab_style);
    ui->official_wordlib_tab_btn->setChecked(true);
    ui->wordlib_table_widget->setProperty("type","wordlib_table");


    m_outputWordlibDlg = new OutputWordlibDlg(this);
    connect(ui->official_wordlib_tab_btn,&QPushButton::clicked,this,&WordlibStackedWidget::switch_to_other_page);
    connect(ui->user_defined_wrodlib_tab_btn,&QPushButton::clicked,this,&WordlibStackedWidget::switch_to_other_page);

    init_wordlib_table_view();

}

void WordlibStackedWidget::init_function_map()
{

    /***********************************导出用户词库**********************************/
    /*******************************************************************************/
    auto user_wordlib_export = [this](){

         QString user_wordlib_path = Utils::GetUserWordlibDirPath().append("user.uwl");
         if(!QFile::exists(user_wordlib_path))
         {
             Utils::NoticeMsgBox("用户词库不存在");
             return;
         }

        QString out_put_path = QFileDialog::getSaveFileName(this, tr("保存用户词库"),
                                                        "user.txt",
                                                        tr("File (*.txt);; uwl wordlib(*.uwl)"));
        if(out_put_path.isEmpty())
        {
             return;
        }
        QFileInfo fileInfo(out_put_path);
        if(fileInfo.suffix().compare("uwl") == 0)
        {
            QFile::copy(user_wordlib_path,out_put_path);
            Utils::NoticeMsgBox("导出成功");
            return;
        }


        QString log_put_path = out_put_path + ".log";

        QString targetTextPath = QDir::toNativeSeparators(out_put_path);
        QString errorTextPath = QDir::toNativeSeparators(log_put_path);

        const wchar_t* txtFilePathWChar = reinterpret_cast<const wchar_t*> (targetTextPath.utf16());
        const wchar_t* uwllibPathWChar = reinterpret_cast<const wchar_t*>(user_wordlib_path.utf16());
        const wchar_t* targetErrorPathWChar = reinterpret_cast<const wchar_t*>(errorTextPath.utf16());

        wchar_t errorParamer[256] = {0};
        _tcscpy(errorParamer, targetErrorPathWChar);

        int ok_count = 0;
        int ret = ExportWordLibrary(uwllibPathWChar, txtFilePathWChar, &ok_count,errorParamer, 1, 0);
        (ret == 1)?Utils::NoticeMsgBox("导出成功"):  Utils::NoticeMsgBox("导出失败");

    };


    /***********************************清空用户词库**********************************/
    /*******************************************************************************/
    auto clear_user_wordlib = [this](){

        if(Utils::NoticeMsgBox("确认清空\n用户词库",this,1) == QDialog::Accepted)
        {
            QString user_wordlib_path = Utils::GetUserWordlibDirPath().append("user.uwl");
            if(QFile::remove(user_wordlib_path))
            {
                Config::Instance()->make_config_work();
                Utils::NoticeMsgBox("用户词库清空成功");
                return;
            }
            else
            {
                 Utils::NoticeMsgBox("用户词库清空失败");
            }
        }

    };

    /***********************************更新系统词库和官方词库**********************************/
    /***************************************************************************************/
    auto update_category_wordlib = [this](){
        ui->update_category_wordlib_btn->setVisible(false);
        if(m_sys_wordlib_has_update){
             m_update_wordlib_thread->start();
        }
        if(m_profess_wordlib_has_update){
            m_profess_wordlib_update_thread->start();
        }
    };


    /***********************************导出官方词库和自定义词库**********************************/
    /*****************************************************************************************/
    auto export_category_wordlib = [this](){
        //检查是否选中了对应的词库
        int selected_num = 0;
        QVector<QString>  can_not_export_wordlib_vector;
        QVector<QString>  can_export_wordlib_vector;
        can_not_export_wordlib_vector.clear();
        can_export_wordlib_vector.clear();
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
            for_each(m_offical_wordlib_info_vector.begin(),m_offical_wordlib_info_vector.end(),count_table_select);
        }
        else if(ui->stackedWidget->currentIndex() == 1)
        {
             for_each(m_user_wordlib_info_vector.begin(),m_user_wordlib_info_vector.end(),count_table_select);
        }

        if(selected_num == 0)
        {
            Utils::NoticeMsgBox("没有可导出的词库");
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
            Utils::NoticeMsgBox("没有可导出的词库");
            return;
        }

        QString selectedFilePath = m_outputWordlibDlg->GetSaveDirPath();
        QString output_type = m_outputWordlibDlg->GetOutputFileType();

        auto output_wordlib_file = [&](QString wordlib_name){
            QString uwl_path = find_wordlibpath_by_name(wordlib_name);
            if(uwl_path.isEmpty()){return;}

            QFileInfo info(uwl_path);
            QString targetTextPath = QDir::toNativeSeparators(selectedFilePath + "/" + info.baseName() + ".txt");
            QString errorTextPath = QDir::toNativeSeparators(selectedFilePath + "/" + info.baseName() + ".log");

            const wchar_t* txtFilePathWChar = reinterpret_cast<const wchar_t*> (targetTextPath.utf16());
            const wchar_t* uwllibPathWChar = reinterpret_cast<const wchar_t*>(uwl_path.utf16());
            const wchar_t* targetErrorPathWChar = reinterpret_cast<const wchar_t*>(errorTextPath.utf16());

            wchar_t errorParamer[256] = {0};
            _tcscpy(errorParamer, targetErrorPathWChar);

            int ok_count = 0;
            int ret = ExportWordLibrary(uwllibPathWChar, txtFilePathWChar, &ok_count,errorParamer, 1, 0);
            if(ret == 0)
            {
                Utils::NoticeMsgBox(QString("%1\n导出失败").arg(wordlib_name));
            }

        };
        auto output_uwl_file = [&](QString wordlib_name){
            QString uwl_path = find_wordlibpath_by_name(wordlib_name);
            if(uwl_path.isEmpty()){return;}

            QFile uwl_file(uwl_path);
            QFileInfo info(uwl_path);
            QString targetTextPath = QDir::toNativeSeparators(selectedFilePath + "/" + info.baseName() + ".uwl");
            uwl_file.copy(targetTextPath);
        };

        if(output_type.compare("词库文件(.uwl)") == 0){
            for_each(can_export_wordlib_vector.begin(),can_export_wordlib_vector.end(),output_uwl_file);
        }
        else if(output_type.compare("文本文件(.txt)") == 0){

            QString file_name_str;
            if(can_not_export_wordlib_vector.size()>0)
            {
                auto combine_str = [&](QString file_name){
                  file_name_str.append(file_name);
                  file_name_str.append(" ");
                };
                for_each(can_not_export_wordlib_vector.begin(),can_not_export_wordlib_vector.end(),combine_str);
                Utils::NoticeMsgBox(QString("%1\n无法导出文本").arg(file_name_str));
                return;
            }

            for_each(can_export_wordlib_vector.begin(),can_export_wordlib_vector.end(),output_wordlib_file);
        }

        Utils::NoticeMsgBox("导出完成");
    };

    /***********************************导入官方词库和自定义词库**********************************/
    /*****************************************************************************************/
    auto import_category_wordlib = [&](){

        QString input_file_path = QFileDialog::getOpenFileName(this, tr("打开需要导入的词库"),
                                                            "",
                                                            tr("wordlib format (*.uwl *.txt)"));
        if(input_file_path.isEmpty())
        {
            return;
        }


        QString target_file_name = QFileInfo(input_file_path).baseName() + ".uwl";
        auto find_pos = std::find_if(m_officail_wordlib_list.cbegin(),m_officail_wordlib_list.cend(),[&](WordlibListInfo fileinfo){
                return fileinfo.fileName == target_file_name;
            });
        bool is_official_wordlib = false;
        if(find_pos != m_officail_wordlib_list.end())
        {
            is_official_wordlib = true;
        }
        if(is_official_wordlib)
        {
            Utils::NoticeMsgBox("文件名称与官方词库冲突");
            return;
        }

        QString target_file_path = Config::Instance()->wordLibDirPath() + QFileInfo(input_file_path).baseName() + ".uwl";
        if(QFile::exists(target_file_path))
        {
            Utils::NoticeMsgBox("文件名称与已有文件冲突");
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

            if (!create_uwl_wordlib_from_text(input_file_path, import_uwl_file_path))
            {
                Utils::WriteLogToFile(QString("when transform txt to uwl error"));
                Utils::NoticeMsgBox(QString("导入%1\n失败").arg(wordlib_type_str));
                return;
            }

            if(!QFile::copy(import_uwl_file_path,target_file_path))
            {
                Utils::NoticeMsgBox(QString("导入%1\n失败").arg(wordlib_type_str));
                return;
            }


            if(add_wordlib_to_container(target_file_path,is_official_wordlib))
            {
                QFile(import_uwl_file_path).remove();
                (is_official_wordlib)?ui->official_wordlib_tab_btn->click() : ui->user_defined_wrodlib_tab_btn->click();
                 Utils::WriteLogToFile("import user wordlib succeed");
                Utils::NoticeMsgBox(QString("导入%1\n成功").arg(wordlib_type_str));


            }
            else
            {
                Utils::WriteLogToFile("when combine user uwl throw error failed");
                QFile(import_uwl_file_path).remove();
                Utils::NoticeMsgBox(QString("导入%1\n失败").arg(wordlib_type_str));

            }
        }
        else if ((wordlib_info.suffix() == "uwl"))
        {
            if(QFile::copy(input_file_path,target_file_path)&&(add_wordlib_to_container(target_file_path,is_official_wordlib)))
            {

                (is_official_wordlib)?ui->official_wordlib_tab_btn->click() : ui->user_defined_wrodlib_tab_btn->click();
                Utils::WriteLogToFile("import user wordlib succeed");
                Utils::NoticeMsgBox(QString("导入%1\n成功").arg(wordlib_type_str));
            }
            else
            {
                Utils::WriteLogToFile("when combine user uwl throw error failed");
                Utils::NoticeMsgBox(QString("导入%1\n失败").arg(wordlib_type_str));

            }
        }

    };

    m_btn_function_map[ui->user_wordlib_export_btn] = user_wordlib_export;
    m_btn_function_map[ui->clear_user_wordlib_btn] = clear_user_wordlib;
    m_btn_function_map[ui->update_category_wordlib_btn] = update_category_wordlib;
    m_btn_function_map[ui->export_category_wordlib_btn] = export_category_wordlib;
    m_btn_function_map[ui->import_category_wordlib_btn] = import_category_wordlib;
}

void WordlibStackedWidget::init_wordlib_thread()
{
    m_wordlib_check_thread = QSharedPointer<SysWordlibCheckThread>(new SysWordlibCheckThread());
    m_profess_wordlib_check_thread = QSharedPointer<ProfessWordlibCheckThread>(new ProfessWordlibCheckThread());

    connect(m_wordlib_check_thread.data(),&SysWordlibCheckThread::updatecoming,this,[&](){
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
    connect(m_profess_wordlib_update_thread.data(),&ProfessWordlibDownloadThread::download_error,this,[&](QString wordlib_name){

        refresh_update_btn_state();
    });
}

void WordlibStackedWidget::init_wordlib_table_view()
{
    m_official_wordlib_model = wordlib_model(new MyTableModel());
    m_official_wordlib_view = wordlib_view(new MyTableView());
    m_official_wordlib_delegate = wordlib_delegate(new Delegate(m_official_wordlib_view.data()));

    m_user_wordlib_model = wordlib_model(new MyTableModel());
    m_user_wordlib_view = wordlib_view(new MyTableView());
    m_user_wordlib_delegate = wordlib_delegate(new Delegate(m_user_wordlib_view.data()));

   m_official_wordlib_view->setItemDelegate(m_official_wordlib_delegate.data());
   m_official_wordlib_view->setModel(m_official_wordlib_model.data());
   m_official_wordlib_view->verticalHeader()->setDefaultSectionSize(40);

   m_user_wordlib_view->setItemDelegate(m_user_wordlib_delegate.data());
   m_user_wordlib_view->setModel(m_user_wordlib_model.data());
   m_user_wordlib_view->verticalHeader()->setDefaultSectionSize(40);

   //智能选中单个单元格
   m_official_wordlib_view->setSelectionBehavior(QAbstractItemView::SelectItems);
   m_official_wordlib_view->setSelectionMode(QAbstractItemView::ExtendedSelection);
   m_user_wordlib_view->setSelectionBehavior(QAbstractItemView::SelectItems);
   m_user_wordlib_view->setSelectionMode(QAbstractItemView::ExtendedSelection);

   QHBoxLayout* wordlib_tab_layout = new QHBoxLayout(ui->official_wordlib_page);
   wordlib_tab_layout->setMargin(0);
   wordlib_tab_layout->addWidget(m_official_wordlib_view.data());

   QHBoxLayout* user_tab_layout = new QHBoxLayout(ui->user_wordlib_page);
   user_tab_layout->setMargin(0);
   user_tab_layout->addWidget(m_user_wordlib_view.data());

   m_official_wordlib_view->setProperty("type","wordlib_item_view");
   m_user_wordlib_view->setProperty("type","wordlib_item_view");


   connect(m_official_wordlib_delegate.data(), &Delegate::wordlib_removed, this, &WordlibStackedWidget::remove_official_wordlib_slot);
   connect(m_user_wordlib_delegate.data(), &Delegate::wordlib_removed, this, &WordlibStackedWidget::remove_user_wordlib_slot);

   connect(m_user_wordlib_delegate.data(), &Delegate::wordlib_state_changed, this, [&](){
       slot_update_selected_wordlib();

   });
   connect(m_user_wordlib_delegate.data(), &Delegate::import_user_wordlib_btn_clicked, this, [&](){
       m_btn_function_map[ui->import_category_wordlib_btn]();
   });
   connect(m_official_wordlib_delegate.data(), &Delegate::wordlib_state_changed, this,[&](){slot_update_selected_wordlib();});

}

bool WordlibStackedWidget::load_official_wordlib_list_from_file()
{
    OFFICIAL_DEFAULT_WORDLIB_LIST.clear();
    QString wordlib_list_path  = QApplication::applicationDirPath() + "\\wordlib_list.json";

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
try
{
    m_offical_wordlib_info_vector.clear();
    m_user_wordlib_info_vector.clear();
    m_custom_wordlib_list.clear();
    load_official_wordlib_list_from_file();
    m_officail_wordlib_list = get_official_wordlib_list();
    config_info temp_config;
    if(Config::Instance()->GetConfigItemByJson("custom_wordlib_list",temp_config))
    {
        m_custom_wordlib_list = temp_config.itemCurrentStrValue.split(";");
    }

    QVector<QString> selected_wordlib_vector = Config::Instance()->GetSelectedWordlibContainer();

    auto is_contains_in_official_wordlib = [=](QString index_wordlib_name)->bool{
         int size = m_officail_wordlib_list.size();
         for(int index=0; index<size;++index){
                if(m_officail_wordlib_list.at(index).fileName == index_wordlib_name)
                {
                    return true;
                }
         }
         return false;
    };

    //为了向旧版本兼容
    bool is_custom_wordlib_changed = false;

    QSet<QString> all_wordlib_set;
    QDir dir(Config::Instance()->wordLibDirPath());
    dir.setFilter(QDir::Files);
    QStringList filterList;
    filterList << "*.uwl";   //设置筛选条件
    QFileInfoList all_uwl_list = dir.entryInfoList(filterList);
    for (int index = 0; index < all_uwl_list.size(); ++index)
    {
        QFileInfo fileInfo = all_uwl_list.at(index);
        all_wordlib_set.insert(fileInfo.fileName());
    }

    for(QString index_wordlib : all_wordlib_set)
    {
        QString index_wordlib_path = Config::Instance()->wordLibDirPath() + index_wordlib;
        bool wordlib_exist = QFile::exists(index_wordlib_path);
        //旧版本用户导入的词库或者一些暂时未列入官方词库的文件
        //暂时列入用户自定义词库
        if(!m_custom_wordlib_list.contains(index_wordlib) && !is_contains_in_official_wordlib(index_wordlib) &&
                wordlib_exist && !SYS_WORLIDB_LIST.contains(index_wordlib))
        {
            m_custom_wordlib_list.append(index_wordlib);
            is_custom_wordlib_changed = true;
        }
        //两个列表里面都有改词库证明之前该词库是用户自定义词库后来变成了官方词库
        //此时将该词库从自定义词库列表中移除
        else if(m_custom_wordlib_list.contains(index_wordlib) && is_contains_in_official_wordlib(index_wordlib) && wordlib_exist)
        {
            m_custom_wordlib_list.removeOne(index_wordlib);
            is_custom_wordlib_changed = true;
        }
    }

    auto load_wordlib_info  = [&](QString file_name,QString view_name = "",QString url = ""){
        //过滤掉中间产生的V3V5词库
        QString file_path = Config::Instance()->wordLibDirPath() + file_name;
        QFileInfo file_info(file_path);
        if (file_info.exists() && file_info.isFile()  && Utils::IsValidWordLibFile(file_info))
        {
            WORDLIB_INFO wordlib_info;
            if(AnalysisUwl(file_path, wordlib_info))
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
        else if(!file_info.exists())
        {
            QSharedPointer<TAB_WORDLIB_INFO> index_info = QSharedPointer<TAB_WORDLIB_INFO>(new TAB_WORDLIB_INFO);
            index_info->current_state = CLOUD_WORDLIB;
            index_info->wordlib_name = view_name;
            index_info->download_url = url;
            index_info->file_name = file_name;
            m_offical_wordlib_info_vector.push_back(index_info);
        }
    };

    auto load_official_wordlib = [&](WordlibListInfo file_info){
        load_wordlib_info(file_info.fileName,file_info.viewName,file_info.download_url);
    };

    //加载对应的用户词库
    auto load_custom_wordlib = [&](QString file_name){
        //过滤掉中间产生的V3V5词库
        if(file_name.isEmpty()){
            return;
        }
        QString file_path = Config::Instance()->wordLibDirPath() + file_name;
        QFileInfo file_info(file_path);
        if (file_info.exists() && file_info.isFile()  && Utils::IsValidWordLibFile(file_info))
        {
            WORDLIB_INFO wordlib_info;
            if(AnalysisUwl(file_path, wordlib_info))
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
    };

    //添加本地已经有的文件
    for_each(m_officail_wordlib_list.begin(),m_officail_wordlib_list.end(),load_official_wordlib);
    for_each(m_custom_wordlib_list.begin(),m_custom_wordlib_list.end(),load_custom_wordlib);

    //添加占位的词库信息,用来在用户词库页签里面导入词库
   QSharedPointer<TAB_WORDLIB_INFO> add_wordlib_func = QSharedPointer<TAB_WORDLIB_INFO>(new TAB_WORDLIB_INFO);
   m_user_wordlib_info_vector.push_back(add_wordlib_func);

    //添加本地没有的文件
    m_official_wordlib_model->set_data(m_offical_wordlib_info_vector);
    m_user_wordlib_model->set_data(m_user_wordlib_info_vector);

    if(is_custom_wordlib_changed)
    {
        slot_update_custom_wordlib_list();
    }

}
catch(exception& e)
{
    Utils::WriteLogToFile(QString("load wordlib throw exception:%1").arg(QString(e.what())));
}

void load_official_wordlib()
{

}

WordlibStackedWidget::ParseTxtFileResult WordlibStackedWidget::parser_wordlib_text_file(QString file_path)
{
     ParseTxtFileResult result;
     if (file_path.isEmpty())
     {
         result.ok = false;
         result.errorMsg = QString::fromLocal8Bit("参数错误，输入文件不可为空！");
         return result;
     }

     QFileInfo inputTxtInfo(file_path);
     if (!inputTxtInfo.exists())
     {
         result.ok = false;
         result.errorMsg = QString::fromLocal8Bit("输入文本文件不存在！");
         return result;
     }

     qint64 fileSize = inputTxtInfo.size();
     if (fileSize < 2)
     {
         result.ok = false;
         result.errorMsg = QString::fromLocal8Bit("输入文件大小非法！");;
         result.importableTxtPath = file_path;
         return result;
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
        if(AnalysisUwl(file_info.filePath(), wordlib_info))
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
try
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
catch(exception& e)
{
    Utils::WriteLogToFile(QString("throw exception when refresh wordlib list:%1").arg(e.what()));
}

void WordlibStackedWidget::slot_on_btn_clicked()
{
    QObject *object = QObject::sender();
    QPushButton *push_button = qobject_cast<QPushButton *>(object);
    if(push_button == ui->user_wordlib_import_btn)
    {
        slot_import_user_wordlib();
    }
    else
    {
         m_btn_function_map[push_button]();
    }

}
void WordlibStackedWidget::ResetConfigInfo()
{
    QStringList config_item_name_list;
    config_item_name_list << "local_wordlib_network_update"
                          << "wordlib_name";


    for(int index=0; index<config_item_name_list.size(); ++index)
    {
        QString currentItem = config_item_name_list.at(index);
        ConfigItemStruct configInfo;
        if(Config::Instance()->GetDefualtConfigItem(currentItem,configInfo))
        {
            Config::Instance()->SetConfigItemByJson(currentItem,configInfo);
        }

    }
}
bool WordlibStackedWidget::create_uwl_wordlib_from_text(const QString &txtFilePath, const QString &uwlFilePath)
try
{
    QFileInfo fileInfo(txtFilePath);
    QString baseName = fileInfo.baseName() + ".uwl";
    QString dirPath = fileInfo.dir().path();
    QString targetUwl = dirPath + "/" + baseName;
    QString errorName = fileInfo.baseName() + ".log";

    QString tmp_dir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString targetError = tmp_dir + "/" + errorName;

    QString install_dir = Config::Instance()->GetInstallDir();
    QString wordlib_tools_path;
    if(install_dir.isEmpty())
    {
        return false;
    }
    else
    {
        wordlib_tools_path = install_dir + "\\WordlibTool.exe";
    }
    if(!QFile::exists(wordlib_tools_path))
    {
        return false;
    }
    QStringList argumentList;
    argumentList << "-create_wordlib" <<txtFilePath << uwlFilePath << targetError;
    qDebug() << argumentList;
    QProcess process;
    process.setProgram(wordlib_tools_path);
    process.setArguments(argumentList);
    process.start();
    int ret = process.waitForFinished(5000);
    if(ret != 1)
    {
        if (ret == -99)
        {
            Utils::NoticeMsgBox("文件过大，词库创建失败");
            return false;
        }
        Utils::NoticeMsgBox("词库创建失败");
        return false;
    }
    return true;
}
catch(std::exception& e)
{
    Utils::WriteLogToFile(QString("when create wordlib throw exception:%1").arg(QString(e.what())));
    Utils::NoticeMsgBox("词库创建失败");
    return false;
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
        Config::Instance()->SaveConfig();

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
    for_each(SYS_WORLIDB_LIST.cbegin(),SYS_WORLIDB_LIST.cend(),insert_system_default);

    auto refresh_selected_wordlib = [&](const QSharedPointer<TAB_WORDLIB_INFO>& tab_info){

        if(tab_info->current_state == CHECKED){
            wordlib_container.push_back(tab_info->file_name);
        }
    };

    for_each(cbegin(m_offical_wordlib_info_vector),cend(m_offical_wordlib_info_vector),refresh_selected_wordlib);
    for_each(cbegin(m_user_wordlib_info_vector),cend(m_user_wordlib_info_vector),refresh_selected_wordlib);
    Config::Instance()->SetSelectedWordlibContainer(wordlib_container);
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

    auto update_custom_wordlib_list =[&](){
        config_info custom_info;
        if(!custom_result_str.isEmpty()){
            custom_info.itemName = "custom_wordlib_list";
            custom_info.itemCurrentStrValue = custom_result_str;
            custom_info.itemType = "string";
            Config::Instance()->SetConfigItemByJson("custom_wordlib_list",custom_info);
        }
    };
    for_each(m_user_wordlib_info_vector.cbegin(),m_user_wordlib_info_vector.cend(),update_user_wordlib_list);
    update_custom_wordlib_list();
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

    if(wordlib_info.suffix() == "txt")
    {

        QString tmp_dir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
        QString import_uwl_file_path = QString("%1/%2.uwl")
                .arg(tmp_dir)
                .arg(wordlib_info.baseName());

        if (!create_uwl_wordlib_from_text(input_file_path, import_uwl_file_path))
        {
            Utils::WriteLogToFile(QString("when transform txt to uwl error"));
            Utils::NoticeMsgBox("导入用户词库失败");
            return;
        }
        QString user_wordlib_path = Utils::GetUserWordlibDirPath().append("user.uwl");
        if(combine_wordlib(user_wordlib_path,import_uwl_file_path))
        {
            Config::Instance()->make_config_work();
            Utils::WriteLogToFile("import user wordlib succeed");
            QFile(import_uwl_file_path).remove();
            Utils::NoticeMsgBox("导入成功");
        }
        else
        {
            Utils::WriteLogToFile("when combine user uwl throw error failed");
            QFile(import_uwl_file_path).remove();
            Utils::NoticeMsgBox("导入失败");

        }
    }
    else if (wordlib_info.suffix() == "uwl")
    {
        WORDLIB_INFO input_wordlib_info;
        if(AnalysisUwl(input_file_path,input_wordlib_info) && !input_wordlib_info.can_be_edit)
        {
            Utils::WriteLogToFile("import user wordlib is invalid");
            Utils::NoticeMsgBox("非可编辑词库\n不可导入");
            return;
        }
        QString user_wordlib_path = Utils::GetUserWordlibDirPath().append("user.uwl");
        if(!QFile::exists(user_wordlib_path))
        {
            QFile::copy(input_file_path,user_wordlib_path);
            Config::Instance()->make_config_work();
            Utils::WriteLogToFile("import user wordlib succeed");
            Utils::NoticeMsgBox("导入成功");
            return;
        }
        else
        {
            if(combine_wordlib(user_wordlib_path,input_file_path))
            {
                Config::Instance()->make_config_work();
                Utils::WriteLogToFile("import user wordlib succeed");
                Utils::NoticeMsgBox("导入成功");
            }
            else
            {
                Utils::WriteLogToFile("when combine user uwl throw error failed");
                Utils::NoticeMsgBox("导入失败");

            }
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
            QString file_path = Config::wordLibDirPath() + index_info.fileName;
            if(QFile(file_path).exists())
            {
                QString file_md5 = Utils::GetFileMD5(file_path);
                if(file_md5 != index_info.md5)
                {
                    int returnCode = NetworkHandler::Instance()->DownloadFileFromWeb(index_info.download_url,Config::Instance()->wordLibDirPath(),
                                                                                     file_path,index_info.fileName);
                    QString new_md5 = Utils::GetFileMD5(file_path);
                    if(returnCode != 0){
                        emit download_error(index_info.viewName);
                        ++error_wordlib_count;
                    }
                }
            }

        };
        for_each(custom_wordlib_vector.cbegin(),custom_wordlib_vector.cend(),check_md5);
        emit download_finished(error_wordlib_count);

    } catch (exception& e) {
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
            QString file_path = Config::wordLibDirPath() + index_info.fileName;
            if(QFile(file_path).exists())
            {
                QString file_md5 = Utils::GetFileMD5(file_path);
                WORDLIB_INFO index_wordlib_info;
                WordlibStackedWidget::AnalysisUwl(file_path,index_wordlib_info);
                if((file_md5 != index_info.md5)&&(index_info.viewName.compare(index_wordlib_info.wordlib_name) == 0))
                {
                    has_update_info_vector.push_back(index_info);
                }
            }

        };
        for_each(custom_wordlib_vector.cbegin(),custom_wordlib_vector.cend(),check_md5);
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
