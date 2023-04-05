#include "wordlibtableitemwidget.h"
#include "ui_wordlibtableitemwidget.h"
#include "utils.h"
#include "networkhandler.h"
#include "wordlibapi.h"
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
        ui->state_btn->setIcon(QIcon(QPixmap(":/image/checked.png").scaled(20,20)));
        m_wordlib_info_ptr->current_state = CHECKED;
        break;
    case WORDLIB_STATE::UN_CHECKED:
        ui->state_btn->setIcon(QIcon(QPixmap(":/image/unchecked.png").scaled(20,20)));
        m_wordlib_info_ptr->current_state = UN_CHECKED;
        break;
    case WORDLIB_STATE::CLOUD_WORDLIB:
        ui->state_btn->setIcon(QIcon(QPixmap(":/image/cloud.png").scaled(20,14)));
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
            (Utils::NoticeMsgBox(QString("确认删除\n%1").arg(m_wordlib_info_ptr->wordlib_name),this,1) == QDialog::Accepted))
    {
        emit wordlib_removed(m_wordlib_info_ptr->file_path);
    }
}

void WordlibTableItemWidget::export_btn_clicked()
{
    QString current_wordlib_path = m_wordlib_info_ptr->file_path;
    if(!QFile::exists(current_wordlib_path))
    {
        Utils::NoticeMsgBox("词库文件不存在",this);
        return;
    }
    QString base_name = QFileInfo(current_wordlib_path).baseName();

   QString out_put_path = QFileDialog::getSaveFileName(this, tr("保存词库"),
                                                   QString("%1.txt").arg(base_name),
                                                   tr("File (*.txt);; uwl wordlib(*.uwl)"));
   if(out_put_path.isEmpty())
   {
        return;
   }
   QFileInfo fileInfo(out_put_path);
   if(fileInfo.suffix().compare("uwl") == 0)
   {
       QFile::copy(current_wordlib_path,out_put_path);
       Utils::NoticeMsgBox("导出成功",this);
       return;
   }
   else if(fileInfo.suffix().compare("txt") == 0)
   {
       WORDLIB_INFO wordlib_info;
       WordlibStackedWidget::AnalysisUwl(current_wordlib_path,wordlib_info);
       if(!wordlib_info.can_be_edit)
       {
           Utils::NoticeMsgBox("当前词库不支持文本导出",this);
           return;
       }

       QString log_put_path = out_put_path + ".log";

       QString targetTextPath = QDir::toNativeSeparators(out_put_path);
       QString errorTextPath = QDir::toNativeSeparators(log_put_path);

       const wchar_t* txtFilePathWChar = reinterpret_cast<const wchar_t*> (targetTextPath.utf16());
       const wchar_t* uwllibPathWChar = reinterpret_cast<const wchar_t*>(current_wordlib_path.utf16());
       const wchar_t* targetErrorPathWChar = reinterpret_cast<const wchar_t*>(errorTextPath.utf16());

       wchar_t errorParamer[256] = {0};
       _tcscpy(errorParamer, targetErrorPathWChar);

       int ok_count = 0;
       int ret = ExportWordLibrary(uwllibPathWChar, txtFilePathWChar, &ok_count,errorParamer, 1, 0);
       (ret == 1)?Utils::NoticeMsgBox("导出成功",this):  Utils::NoticeMsgBox("导出失败",this);
   }



}

void WordlibTableItemWidget::download_current_wordlib()
try
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
        for_each(list_vector.cbegin(),list_vector.cend(),get_url);
    }
    else
    {
        Utils::NoticeMsgBox("下载官方词库失败",this);
        return;
    }

    if(m_download_thread->isRunning())
    {
        Utils::NoticeMsgBox("下载中请稍等...",this);
        return;
    }
    if(download_url.isEmpty() || file_name.isEmpty())
    {
        Utils::NoticeMsgBox("该词库暂时无法下载",this);
        return;
    }
    else if(!download_url.isEmpty() && !m_download_thread->isRunning())
    {
        m_download_thread->set_file_save_info(download_url,Config::Instance()->wordLibDirPath(),file_name);
        m_download_thread->start();
    }
}
catch(exception& e)
{
    Utils::WriteLogToFile(QString("throw exception when download wordlib:%1").arg(e.what()));
}

void WordlibTableItemWidget::download_finished(QString file_path)
{
    Utils::WriteLogToFile("download profess wordlib finished");
    if(m_wordlib_info_ptr->current_state == CLOUD_WORDLIB)
    {
        Utils::WriteLogToFile("download profess wordlib finished");
        WORDLIB_INFO wordlib_info;
        if(WordlibStackedWidget::AnalysisUwl(file_path, wordlib_info))
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
    Utils::NoticeMsgBox("下载失败",this);
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


ProfessWordlibThread::ProfessWordlibThread(QString url, QString save_dir, QString file_name):
    m_url(url),m_save_dir(save_dir),m_file_name(file_name)
{

}

void ProfessWordlibThread::run()
try
{
    QString file_path;
    int returnCode = NetworkHandler::Instance()->DownloadFileFromWeb(m_url,m_save_dir,file_path,m_file_name);
    if(returnCode == 0){
        emit download_finished(m_save_dir + m_file_name);
        Utils::WriteLogToFile("download profess wordlib succeed");
    }
    else
    {
        Utils::WriteLogToFile("download profess wordlib failed");
        emit download_error(returnCode);
    }
}
catch(exception& e)
{
    Utils::WriteLogToFile(QString("throw exception in download thread:%1").arg(e.what()));
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

