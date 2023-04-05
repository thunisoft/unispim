#ifndef TRAYCONTROLLER_H
#define TRAYCONTROLLER_H

#include "TrayWindow.h"
#include <pthread.h>
#include <atomic>

enum thread_state
{
    Unrunning = 0,
    Running,
    Stopped
};

typedef enum _HIDE_MAINWINDOW {
    HM_SHOW = 0,
    HM_AUTO = 1,
    HM_HIDE_WHEN_TRAY_AVAILABLE = 2,
    HM_HIDE = 3
} HIDE_MAINWINDOW;


class TrayController
{
public:
    TrayController(ShowMenuCallback show_menu_callback, void* main_controller_pointer);
    ~TrayController();

    void show();
    void hide();
    bool isVisible();
    void SetChangeIMCallBack(ChangeIMCallback input_callback);
private:

private:
    UI* m_ui;
    TrayWindow* m_tray_window;
    pthread_t* m_window_listener;
    bool m_is_visible;

//    ShowMenuCallbackHoleder m_callback_holder;
    ShowMenuCallback m_show_menu_callback;
    ChangeIMCallback m_change_im_callback;
    void* m_main_controller_pointer;
};

#endif // TRAYCONTROLLER_H
