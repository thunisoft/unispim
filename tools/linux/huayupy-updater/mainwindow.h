#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include "updatewidget.h"
#include "updatetip.h"
#include "config.h"
#include "define.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0, UpdateTipPosition position = CENTER);
    ~MainWindow();
public slots:
    void OnCancelButtonClicked();
    void OnUpdateButtonClicked();
    void OnExit();
private:
    void closeEvent(QCloseEvent *event);
public:
   static QSslConfiguration GetQsslConfig();
private:
    UpdateWidget *m_updateWidget;
    UpdateTip *m_updateTip;
signals:
    void SignalDownloadPacakge();

private:
    //void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

    QPoint m_last_mouse_position;
    bool m_move_widget_flag;
};

#endif // MAINWINDOW_H
