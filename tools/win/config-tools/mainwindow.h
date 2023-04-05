#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QCloseEvent>
#include <QMoveEvent>
#include <QTime>
#include <QMetaType>
#include <QElapsedTimer>
#include <QFutureWatcher>
#include <QNativeGestureEvent>
#include <QGraphicsDropShadowEffect>
#include <Windows.h>
#include "commondef.h"
#include "ui_mainwindow.h"
#include "customize_ui/customize_mainwindow.h"

enum PAGE_TYPE
{
    BASIC_PAGE,
    ADVANCED_PAGE,
    SKIN_PAGE,
    WORDLIB_PAGE,
    LOGIN_PAGE,
    ABOUT_PAGE,
    UPDATE_PAGE,
    FEEDBACK_PAGE
};

class AboutStackedWidget;
class CheckFailedStackedWidget;
class AdvancedConfigStackedWidget;
class BasicConfigStackedWidget;
class SkinConfigStackedWidget;
class WordlibStackedWidget;
class NewEnrollStackedWidget;
class PersonAccountStackedWidget;
class FeedbackStackedWidget;
class DpiAdaptor;
class SharedMemoryCheckThread;
class MainWindow : public CustomizeMainWindow
{ 
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

static bool SendMsgToEngine();

private:
    void Init();
    void InitStackWidget();
    void SetUpCustomizeUI();

    void SetUpCloseButton();

    void ChangeVisibleWidget(int index);
    void RegisterSlots();


    void InitBtnIcon();

    void OnBtnStateChanged();
    /**@brief 通过读取文件向主窗口中添加提示红点
   */
    void AddNoticeInfoToMainwindow();
    int GetScreenSizeFactor(int screenArea);

public slots:
    void ChangeUserLoginState(bool flag);
    void LoadChildConfigPage();
    void ResolutionChanged();
    void SlotInformIMEUpdateConfig();

private:
    Ui::MainWindow *ui;
    AboutStackedWidget* m_aboutStack;

    CheckFailedStackedWidget* m_checkFailedStack;
    AdvancedConfigStackedWidget* m_advancedConfigStack;
    BasicConfigStackedWidget* m_basicConfigStack;
    SkinConfigStackedWidget* m_skinConfigStack;
    WordlibStackedWidget* m_wordlibStackedWidget;
    NewEnrollStackedWidget* m_newEnrollStackedWidget;
    PersonAccountStackedWidget* m_personAccountStackedWidget;
    FeedbackStackedWidget* m_feedbackStackedWidget;
    DpiAdaptor* m_dpiAdptor;

    QGraphicsDropShadowEffect* m_shadowEffect;
private slots:
    void OnCloseButtonClicked();

public slots:
    void OnConfigButtonClicked();
    void OnWordlibButtonClicked();
    void OnSkinButtonClicked();
    void OnAccountButtonClicked();
    void OnAboutButtonClicked();
    void OnFeedbackButtonClicked();

    void OnUserLoginedSucceed();
    void OnUserLogout();

    void SwitchToCheckFailedWidget();
    void SwitchToAboutStack();
    void OnAdvancedButtonClicked();

    void OnUpdateTheWidgetConfig();


    void OnApllyBtnClicked();
    void OnSaveBtnClicked();
    void OnResetBtnClicked();

    void SetWindowShowOnTop();
    void SlotMinimizeWindow();
    void OnUpdateTheUserConfigFile();
    void OnNotifyTheEngine();

    void MainWindowShowNormalSlot();

    void ResetAllTheWidght();
    void SlotHideTheMainWindow();

    void ShowAboutVersionCheckWindow();
    void click_index_page(int click_page);
    void slot_on_configfile_update_finished();

public slots:
    /**@brief 给对应的控件添加类似于微信的提示红点
   * @param[in]  push_btn 需要添加提示信息的控件按钮
   */
    void SlotDrawNoticeFlagToWidget(QPushButton* push_btn,bool isAddFalg = true);

    void slot_show_edit_hotkey();
signals:
    void CandidateCountChanged(int currentIndex);


private:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

protected:
    void closeEvent(QCloseEvent* event);
private:
    QPoint m_last_mouse_position;
    bool m_move_widget_flag;

private:
    int m_screenPos;
    screenOptions m_screenOption;
    QElapsedTimer m_countTime;
    QLabel* m_notice_label = NULL;
    SharedMemoryCheckThread* m_memoryCheckThread;
    QFutureWatcher<int>* m_config_update_watcher;
};

#endif // MAINWINDOW_H
