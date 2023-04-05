#ifndef WORDLIBTABLEITEMWIDGET_H
#define WORDLIBTABLEITEMWIDGET_H

#include <QWidget>
#include <QEvent>
#include <QDialog>
#include <QSharedPointer>
#include <QMouseEvent>
#include <QMouseEvent>
#include <QThread>
#include <QHoverEvent>
#include <QEvent>
#include "mywordlibtableview.h"

namespace Ui {
class WordlibTableItemWidget;
}

class ProfessWordlibThread final : public QThread
{
    Q_OBJECT
public:
    ProfessWordlibThread() = default;
    ~ProfessWordlibThread() = default;
    explicit ProfessWordlibThread(QString url, QString save_dir,QString file_name);

public:
    void run() override;
    QString get_file_path();
    void set_file_save_info(QString url,QString file_dir, QString file_name);

signals:
    void download_finished(QString file_path);
    void download_error(int);

 private:
    QString m_url;
    QString m_save_dir;
    QString m_file_name;
};

class WordlibTableItemWidget final : public QWidget
{
    Q_OBJECT
    using tab_wordlib_info_ptr = TAB_WORDLIB_INFO*;
    using download_thread_ptr = QSharedPointer<ProfessWordlibThread>;
public:
    explicit WordlibTableItemWidget(QWidget *parent = 0);
    ~WordlibTableItemWidget(); ///析构函数中禁止析构m_worlib_info_ptr;

public slots:
    void set_btn_state(WORDLIB_STATE state);
    void set_wordlib_info(tab_wordlib_info_ptr wordlib_info);
    void state_btn_clicked();
    void delete_btn_clicked();
    void export_btn_clicked();
    void download_current_wordlib();
    void download_finished(QString file_path);
    void download_error(int return_code);

protected:
    void mousePressEvent(QMouseEvent*event);
protected:
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);
    bool event(QEvent * e);

    void hoverEnter(QHoverEvent * event);
    void hoverLeave(QHoverEvent * event);
    void hoverMove(QHoverEvent * event);

signals:
    void wordlib_removed(QString file_path);
    void wordlib_state_changed();
    void wordlib_download(QString wordlib_name);
private:
    Ui::WordlibTableItemWidget *ui;
    tab_wordlib_info_ptr m_wordlib_info_ptr;   ///其它的地方有智能指针调用此处禁止析构
    bool m_is_selected =false;
    download_thread_ptr m_download_thread;
};

#endif // WORDLIBTABLEITEMWIDGET_H
