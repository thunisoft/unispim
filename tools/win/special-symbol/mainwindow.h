#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMouseEvent>
#include <QQueue>
#include <QLabel>
#include <QCloseEvent>
#include <windowsx.h>
#include <QFocusEvent>
#include <QFocusEvent>
#include "sharedmemorycheckthread.h"
#include "characterwidget.h"



namespace Ui {
class MainWindow;
}

class MainSymbolStackedWidget;
class SearchFailed;
class SearchResultWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public:
    void SetWindowShowOnTop();

private Q_SLOTS:
    void SlotCloseWindow();
    void SlotMinimizeTheWindow();


protected:

    void closeEvent(QCloseEvent* event);
    virtual void focusOutEvent(QFocusEvent* e);


private slots:
    void SendSymbolToOtherWindowSlot(QString symbol);
    void SetCurrentWindowActive();

    void SearchForResultSlot();

    void SwitchToMainWindow();

    void MainWindowShowNormalSlot();
private:
    void InitInputService();
    void UnInitInputService();
    void UpdateRecentShow();


    void ResetTheHistorySymbol(QString inputStr);

Q_SIGNALS:
    void commit(QString str);

private:
    Ui::MainWindow *ui;

private:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
    QPoint m_last_mouse_position;
    bool m_move_widget_flag;
    MainSymbolStackedWidget* m_mainSymbolStackedWidget;
    SearchFailed* m_searchFailedWidget;
    SearchResultWidget* m_searchResultWidght;
    HWND m_preWindowHwnd = (HWND)0;
    SharedMemoryCheckThread* m_memoryCheckThread;
};

#endif // MAINWINDOW_H
