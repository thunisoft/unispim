#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QFutureWatcher>
#include <QFutureWatcher>
#include <QtConcurrent>
#include <QFileInfo>
#include "commondef.h"
#include "../public/utils.h"
#include "ui_mainwindow.h"
#include "customize_ui/customize_mainwindow.h"



class AboutStackedWidget;
class ModeStackedWidget;
class ConfigStackedWidget;
class AccountStackedWidget;
class WordlibMainWidget;
class CheckFailedStackedWidget;
class RegisterStackedWidget;
class EnrollStackedWidget;
class ResetPasswdStackedWidget;
class FeedbackStackedWidget;
class wubistackedwidght;
class SkinStackedWidget;
class AdvancedConfigStackedWidget;
class BasicConfigStackedWidget;
class WordlibStackedWidget;
class PersonAccountStackedWidget;
class MainWindow : public CustomizeMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    void Init();
    void InitStackWidget();
    void SetUpCustomizeUI();
    void SetUpLogo();
    void SetUpStackWidgetOptionButtonSlot();
    void SetUpCloseButton();
    void SetUpMinmizeWindowButton();
    void SetUpOptionButton();
    void ChangeVisibleWidget(int index);
    void RegisterSlots();
    void OnBtnStateChanged();
    void SetUpApplyButton();
    void SetStyleSheet();
    void DrawNoticeFlagToWidget(QPushButton *push_btn, bool isAddFlag = true);
    int CheckNewVersion();

private:
    Ui::MainWindow *ui;
    AboutStackedWidget* m_aboutStack = nullptr;
    WordlibStackedWidget* m_wordlibStack = nullptr;
    CheckFailedStackedWidget* m_checkFailedStack = nullptr;
    RegisterStackedWidget* m_registerStack = nullptr;
    EnrollStackedWidget* m_enrollStack= nullptr;
    ResetPasswdStackedWidget* m_resetPasswdStack= nullptr;
    FeedbackStackedWidget* m_feedbackStack= nullptr;
    wubistackedwidght* m_wubiStack= nullptr;
    SkinStackedWidget* m_skinConfigStack= nullptr;
    AdvancedConfigStackedWidget* m_advancedStack= nullptr;
    BasicConfigStackedWidget* m_basicStack= nullptr;
    PersonAccountStackedWidget* m_personinfo_stack = nullptr;
    QLabel *m_notice_label = nullptr;
    QFutureWatcher<int>* m_version_check_watcher;
    QFutureWatcher<int>* m_user_wordlib_watcher;


private slots:
    void OnCloseButtonClicked();
    void SlotResetTheWindowConfig();

public slots:
    void OnWordlibButtonClicked();
    void OnAccountButtonClicked();
    void OnAboutButtonClicked();
    void OnFeedBackButtonClicked();
    void OnUserLoginedSlot();
    void OnUserLogoutSlot();
    void SwitchToReigisterSlot();
    void SwitchToLoginSlot();
    void SwitchToAccountSlot();
    void SwitchToResetPasswdSlot();
    void SwitchToCheckFailedWidget();
    void SwitchToAboutStack();
    void OnWubiButtonClicked();
    void ToShow();
    void OnSkinOptionBtnClicked();
    void OnBasicOptionBtnClicked();
    void OnAdvancedOptionBtnClicked();
    void SlotMinimizeWindow();
    int UpdateUserWordlib(QString user_uwl_path,QString loginid);
    int quitUploadUserlib();
    void NotifyReloadUserWordlib();
    void OnVesionCheckFinished();
    void OnWordlibUpdateFinished();
    void UpdateAllStackedWidget();
    void UploadUserWorild();
    void ExitCurrentApp();
    void OnSideButtonClick(QString btn_name);

signals:
    void CandidateCountChanged(int currentIndex);
};

#endif // MAINWINDOW_H
