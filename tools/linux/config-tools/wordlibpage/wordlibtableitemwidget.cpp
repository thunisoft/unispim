#include "wordlibtableitemwidget.h"
#include "ui_wordlibtableitemwidget.h"
#include "networkhandler.h"
#include "../public/configmanager.h"
#include "myutils.h"
#include <thread>
#include <QFileDialog>


WordlibTableItemWidget::WordlibTableItemWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WordlibTableItemWidget)
{
    ui->setupUi(this);
    ui->delete_btn->setProperty("type","wordlib_item_delete");
    ui->widget_frame->setProperty("type","wordlib_item_frame");
    ui->state_btn->setProperty("type","wordlib_item_state");
    ui->export_btn->setProperty("type","wordlib_item_export");
    ui->state_btn->setCursor(Qt::PointingHandCursor);
    ui->delete_btn->setCursor(Qt::PointingHandCursor);
    ui->export_btn->setCursor(Qt::PointingHandCursor);
    this->setAttribute(Qt::WA_Hover,true);
    ui->delete_btn->setVisible(false);
    ui->export_btn->setVisible(false);
    ui->delete_btn->setToolTip("删除");
    ui->export_btn->setToolTip("导出");
    ui->delete_btn->setStyleSheet("QToolTip{font-size:14px;}");
    ui->export_btn->setStyleSheet("QToolTip{font-size:14px;}");
    ui->wordlib_name_label->setStyleSheet("QToolTip{font-size:14px;}");
    ui->wordlib_name_label->setAttribute(Qt::WA_TranslucentBackground,true);

    m_download_thread = QSharedPointer<ProfessWordlibThread>(new ProfessWordlibThread());
    connect(m_download_thread.data(),&ProfessWordlibThread::download_finished,this,&WordlibTableItemWidget::download_finished);
    connect(m_download_thread.data(),&ProfessWordlibThread::download_error,this,&WordlibTableItemWidget::download_error);

    connect(ui->state_btn, &QPushButton::clicked,this,&WordlibTableItemWidget::state_btn_clicked);
    connect(ui->delete_btn,&QPushButton::clicked,this,&WordlibTableItemWidget::delete_btn_clicked);
    connect(ui->export_btn,&QPushButton::clicked,this,&WordlibTableItemWidget::export_btn_clicked);

}

WordlibTableItemWidget::~WordlibTableItemWidget()
{
    delete ui;
}

void WordlibTableItemWidget::set_btn_state(WORDLIB_STATE state)
{

    switch (state) {
    case WORDLIB_STATE::CHECKED:
        ui->state_btn->setStyleSheet("QPushButton{min-width:16px;min-height:16px;height:16px;width:16px;image:url(:/image/checked.svg);}");
        m_wordlib_info_ptr->current_state = CHECKED;
        break;
    case WORDLIB_STATE::UN_CHECKED:
        ui->state_btn->setStyleSheet("QPushButton{min-width:16px;min-height:16px;height:16px;width:16px;image:url(:/image/unchecked.svg);}");
        m_wordlib_info_ptr->current_state = UN_CHECKED;
        break;
    case WORDLIB_STATE::CLOUD_WORDLIB:
        ui->state_btn->setStyleSheet("QPushButton{min-width:20px;min-height:16px;height:16px;width:20px;image:url(:/image/cloud.png);}");
        m_wordlib_info_ptr->current_state = CLOUD_WORDLIB;
        break;
    }
    emit wordlib_state_changed();
}

void WordlibTableItemWidget::set_wordlib_info(tab_wordlib_info_ptr wordlib_info)
{
    if(wordlib_info == nullptr)
    {
        return;
    }
    m_wordlib_info_ptr  = wordlib_info;
    ui->wordlib_name_label->setText(m_wordlib_info_ptr->wordlib_name);
    set_btn_state(m_wordlib_info_ptr->current_state);
    QString tool_tip = QString("%1\n词条数:%2\n作者:%3\n更新时间:%4").arg(m_wordlib_info_ptr->wordlib_name)
            .arg(m_wordlib_info_ptr->wordlib_item_count).arg(m_wordlib_info_ptr->autor_name)
            .arg(m_wordlib_info_ptr->update_time);

    if(wordlib_info->current_state != WORDLIB_STATE::CLOUD_WORDLIB)
    {
        ui->wordlib_name_label->setToolTip(tool_tip);
    }
}

void WordlibTableItemWidget::state_btn_clicked()
{
    if(m_wordlib_info_ptr->current_state == CLOUD_WORDLIB)
    {
        //下载更新词库
        download_current_wordlib();
    }
    else if(m_wordlib_info_ptr->current_state == CHECKED)
    {
        set_btn_state(UN_CHECKED);
    }
    else if(m_wordlib_info_ptr->current_state == UN_CHECKED)
    {
        set_btn_state(CHECKED);
    }

}

void WordlibTableItemWidget::delete_btn_clicked()
{

    if(m_wordlib_info_ptr &&
            (TOOLS::Messgebox::notice_msg(QString("确认删除\n%1").arg(m_wordlib_info_ptr->wordlib_name),NULL,1) == QDialog::Accepted))
    {
        emit wordlib_removed(m_wordlib_info_ptr->file_path);
    }
}

void WordlibTableItemWidget::export_btn_clicked()
{
    QString current_wordlib_path = m_wordlib_info_ptr->file_path;
    if(!QFile::exists(current_wordlib_path))
    {
        TOOLS::Messgebox::notice_msg("词库文件不存在");
        return;
    }
    QString base_name = QFileInfo(current_wordlib_path).baseName();
    QString out_put_path = get_output_file_path(base_name);

    if(out_put_path.isEmpty())
    {
        return;
    }
    QFileInfo fileInfo(out_put_path);
    if(fileInfo.suffix().compare("uwl") == 0)
    {
        QFile::copy(current_wordlib_path,out_put_path);
        TOOLS::Messgebox::notice_msg("导出成功");
        return;
    }
    else if(fileInfo.suffix().compare("txt") == 0)
    {
        WORDLIB_INFO wordlib_info;
        TOOLS::WordlibUtils::get_uwl_fileinfo(current_wordlib_path,wordlib_info);
        if(!wordlib_info.can_be_edit)
        {
            TOOLS::Messgebox::notice_msg("当前词库不支持文本导出");
            return;
        }

        QString targetTextPath = QDir::toNativeSeparators(out_put_path);
        int ret = TOOLS::WordlibUtils::export_uwl_wordlib_to_text(current_wordlib_path,targetTextPath);
        (ret == 1)?TOOLS::Messgebox::notice_msg("导出成功"):  TOOLS::Messgebox::notice_msg("导出失败");
    }

}

void WordlibTableItemWidget::download_current_wordlib()
{
    QVector<WordlibListInfo> list_vector;
    QString download_url;
    QString file_name;
    if(NetworkHandler::Instance()->get_profess_wordlib_list(list_vector) == 0)
    {
        auto get_url = [&](WordlibListInfo input_info){
            QString wordlib_name = input_info.viewName;
            if((wordlib_name == m_wordlib_info_ptr->wordlib_name))
            {
                download_url = input_info.download_url;
                file_name = input_info.fileName;
            }

        };
        std::for_each(list_vector.cbegin(),list_vector.cend(),get_url);
    }
    else
    {
        TOOLS::Messgebox::notice_msg("下载官方词库失败");
        return;
    }

    if(m_download_thread->isRunning())
    {
        TOOLS::Messgebox::notice_msg("下载中请稍等...");
        return;
    }
    if(download_url.isEmpty() || file_name.isEmpty())
    {
        TOOLS::Messgebox::notice_msg("该词库暂时无法下载");
        return;
    }
    else if(!download_url.isEmpty() && !m_download_thread->isRunning())
    {
        QString local_wordlib_dir = TOOLS::PathUtils::get_local_wordlib_path() + "cloud/";
        QDir dir(local_wordlib_dir);
        if(!dir.exists())
        {
            dir.mkpath(local_wordlib_dir);
        }
        m_download_thread->set_file_save_info(download_url,local_wordlib_dir,file_name);
        m_download_thread->start();
    }
}

void WordlibTableItemWidget::download_finished(QString file_path)
{
    TOOLS::LogUtils::write_log_to_file("download profess wordlib finished");
    if(m_wordlib_info_ptr->current_state == CLOUD_WORDLIB)
    {
        Utils::WriteLogToFile("download profess wordlib finished");
        WORDLIB_INFO wordlib_info;
        if(TOOLS::WordlibUtils::get_uwl_fileinfo(file_path, wordlib_info))
        {
            m_wordlib_info_ptr->wordlib_name = wordlib_info.wordlib_name.trimmed();
            m_wordlib_info_ptr->wordlib_item_count = wordlib_info.words_count;
            m_wordlib_info_ptr->autor_name = wordlib_info.author;
            m_wordlib_info_ptr->update_time = wordlib_info.wordlib_last_modified_date;
            m_wordlib_info_ptr->file_path = file_path;
            m_wordlib_info_ptr->file_name = QFileInfo(file_path).fileName();
            m_wordlib_info_ptr->can_be_edit = wordlib_info.can_be_edit;
            m_wordlib_info_ptr->is_official_wordlib = true;

        }
        set_btn_state(CHECKED);
        QString tool_tip = QString("%1\n词条数:%2\n作者:%3\n更新时间:%4").arg(wordlib_info.wordlib_name)
                .arg(wordlib_info.words_count).arg(wordlib_info.author)
                .arg(wordlib_info.wordlib_last_modified_date);
        ui->wordlib_name_label->setToolTip(tool_tip);
    }
}

void WordlibTableItemWidget::download_error(int return_code)
{
    Q_UNUSED(return_code);
}


void WordlibTableItemWidget::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
}

bool WordlibTableItemWidget::event(QEvent * e)
{
    switch(e->type())
    {
    case QEvent::HoverEnter:
        hoverEnter(static_cast<QHoverEvent*>(e));
        return true;
        break;
    case QEvent::HoverLeave:
        hoverLeave(static_cast<QHoverEvent*>(e));
        return true;
        break;
    case QEvent::HoverMove:
        hoverMove(static_cast<QHoverEvent*>(e));
        return true;
        break;
    default:
        break;
    }
    return QWidget::event(e);
}

void WordlibTableItemWidget::enterEvent(QEvent * e)
{
    Q_UNUSED(e);
}

void WordlibTableItemWidget::leaveEvent(QEvent * e)
{
    Q_UNUSED(e);
}

void WordlibTableItemWidget::hoverEnter(QHoverEvent * event)
{
    Q_UNUSED(event);
    if(m_wordlib_info_ptr->current_state != WORDLIB_STATE::CLOUD_WORDLIB)
    {
        ui->delete_btn->setVisible(true);
        if(!m_wordlib_info_ptr->is_official_wordlib)
        {
            ui->export_btn->setVisible(true);
        }
        QStringList offical_wordlib_list = ConfigManager::Instance()->get_defaut_official_wordlib_list();
        if(offical_wordlib_list.contains(m_wordlib_info_ptr->wordlib_name))
        {
            ui->delete_btn->setVisible(false);
        }
    }
}

void WordlibTableItemWidget::hoverLeave(QHoverEvent * event)
{
    Q_UNUSED(event);
    ui->delete_btn->setVisible(false);
    ui->export_btn->setVisible(false);
}

void WordlibTableItemWidget::hoverMove(QHoverEvent * event)
{
    Q_UNUSED(event);
}

QString WordlibTableItemWidget::get_output_file_path(QString base_name)
{

    QFileDialog *fileDialog = new QFileDialog(this);
    fileDialog->setWindowTitle("保存词库");
    fileDialog->setAcceptMode(QFileDialog::AcceptSave);
    fileDialog->setFileMode(QFileDialog::AnyFile);
    fileDialog->setViewMode(QFileDialog::Detail);

    fileDialog->setDirectory(".");
    fileDialog->setNameFilters(QStringList() << "File (*.txt)" << "uwl wordlib(*.uwl)");
    fileDialog->selectFile(QString("%1.%2").arg(base_name).arg("txt"));

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


ProfessWordlibThread::ProfessWordlibThread(QString url, QString save_dir, QString file_name):
    m_url(url),m_save_dir(save_dir),m_file_name(file_name)
{

}

void ProfessWordlibThread::run()
try
{
    QString file_save_path = m_save_dir + m_file_name;
    int returnCode = NetworkHandler::Instance()->DownloadFileFromWeb(m_url,file_save_path);
    if(returnCode == 0){
        emit download_finished(m_save_dir + m_file_name);
    }
    else
    {
        emit download_error(returnCode);
    }
}
catch(std::exception& e)
{
    //Utils::WriteLogToFile(QString("throw exception in download thread:%1").arg(e.what()));
}

QString ProfessWordlibThread::get_file_path()
{
    return m_save_dir + m_file_name;
}

void ProfessWordlibThread::set_file_save_info(QString url, QString file_dir, QString file_name)
{
    m_save_dir = file_dir;
    m_file_name = file_name;
    m_url = url;
}

