// X11 实现的 TrayIcon
// 只有银河麒麟、中科方德 v10.1 使用，TrayIcon 不被收纳
#include "maincontroller.h"
#include "traycontroller.h"
#include "tray.h"



void* WindowEventHandler(void* trayWindow);
void InitAtom(UI* ui);

TrayController::TrayController(ShowMenuCallback show_menu_callback, void* main_controller_pointer)
{
    m_window_listener = NULL;
    m_tray_window = NULL;
    m_show_menu_callback = show_menu_callback;
    m_main_controller_pointer = main_controller_pointer;

    //m_tray_window = new TrayWindow;
    //m_tray_window->show_menu_callback = show_menu_callback;
    //m_tray_window->main_controller_pointer = main_controller_pointer;

    m_is_visible = false;
}

TrayController::~TrayController()
{
    delete m_window_listener;
    delete m_tray_window;
    if(m_tray_window->owner)
    {
        delete m_tray_window->owner;
        m_tray_window->owner = NULL;
    }
    m_tray_window = NULL;
    m_window_listener = NULL;
}

void* WindowEventHandler(void* trayWindow)
{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    TrayWindow* tray_window = (TrayWindow*)trayWindow;

    while(1)
    {
        pthread_testcancel();
        XEvent event = {0};
        XNextEvent(tray_window->owner->dpy, (XEvent *)&event);
        switch(event.type)
        {
        case MotionNotify:
            if (event.xexpose.window == tray_window->window) {
                TrayWindowDraw(tray_window);
            }
            break;
        case Expose:
            if (event.xexpose.window == tray_window->window) {
                TrayWindowDraw(tray_window);
            }
            break;
        case ConfigureNotify:
            if (tray_window->window == event.xconfigure.window) {
                int size = event.xconfigure.height;
                if (size != tray_window->size) {
                    tray_window->size = size;
                    XResizeWindow(tray_window->owner->dpy, tray_window->window, size, size);
                    XSizeHints size_hints;
                    size_hints.flags = PWinGravity | PBaseSize;
                    size_hints.base_width = tray_window->size;
                    size_hints.base_height = tray_window->size;
                    XSetWMNormalHints(tray_window->owner->dpy, tray_window->window, &size_hints);
                }

                TrayWindowDraw(tray_window);
            }
            break;
        case ButtonPress: {
            if (event.xbutton.window == tray_window->window) {
                switch (event.xbutton.button) {
                case Button1:
                    if(tray_window->main_controller_pointer){

                        if(tray_window->main_controller_pointer && tray_window->change_im_callback)
                            tray_window->change_im_callback(tray_window->main_controller_pointer);

                    }
                    break;
                case Button3: {
//                    XlibMenu *mainMenuWindow = classicui->mainMenuWindow;
//                    mainMenuWindow->anchor = MA_Tray;
//                    mainMenuWindow->trayX = event->xbutton.x_root - event->xbutton.x;
//                    mainMenuWindow->trayY = event->xbutton.y_root - event->xbutton.y;                    
//                    XlibMenuShow(mainMenuWindow);
                    //showMenu
                    int x = event.xbutton.x_root - event.xbutton.x;
                    int y = event.xbutton.y_root - event.xbutton.y;
                    if(tray_window->main_controller_pointer && tray_window->show_menu_callback)
                        tray_window->show_menu_callback(tray_window->main_controller_pointer, x, y);
                }
                break;
                }

            }
        }
        break;
//    case DestroyNotify:
//        if (event.xdestroywindow.window == tray_window->dockWindow) {
//            tray_window->dockWindow = TrayGetDock(tray_window);
//            tray_window->bTrayMapped = False;
//            TrayWindowRelease(tray_window);
//            if (tray_window->dockWindow != None) {
//                TrayWindowInit(tray_window);
//            }
//        }
//        break;
    default :
        break;
        }
    }
    return 0;
}

void TrayController::show()
{
    if(isVisible())
        return;
    UI* ui = new UI;
    ui->dpy = XOpenDisplay(NULL);
    ui->iScreen = DefaultScreen(ui->dpy);
    InitAtom(ui);
    if(!m_tray_window)
    {
        m_tray_window = new TrayWindow;
        m_tray_window->window = None;
        m_tray_window->dockWindow = None;
        m_tray_window->show_menu_callback = m_show_menu_callback;
        m_tray_window->main_controller_pointer = m_main_controller_pointer;
        m_tray_window->change_im_callback = m_change_im_callback;
    }
    TrayWindowCreate(ui, m_tray_window);
    TrayWindowInit(m_tray_window);

    m_window_listener = new pthread_t;

    if(pthread_create(m_window_listener, NULL, WindowEventHandler, (void*)m_tray_window))
    {
        //create thread error;
        return;
    }    
    pthread_detach(*m_window_listener);
    m_is_visible = true;
}

void TrayController::hide()
{
   if(!isVisible())
        return;
   if(m_window_listener)
   {
       pthread_cancel(*m_window_listener);
       delete m_window_listener;
       m_window_listener = NULL;

       TrayWindowRelease(m_tray_window);
       XFlush(m_tray_window->owner->dpy);

       delete m_tray_window->owner;
   }

   m_is_visible = false;
}

bool TrayController::isVisible()
{
    return m_is_visible;
}

void TrayController::SetChangeIMCallBack(ChangeIMCallback input_callback)
{
    m_change_im_callback = input_callback;
}

void InitAtom(UI* ui)
{
#define CMPMGR_PREFIX "_NET_WM_CM_S"
    char atom_name[sizeof(CMPMGR_PREFIX) + FCITX_INT64_LEN] = CMPMGR_PREFIX;
    sprintf(atom_name + strlen(CMPMGR_PREFIX), "%d", ui->iScreen);
#undef CMPMGR_PREFIX
    char *atom_names[] = {
        "_NET_WM_WINDOW_TYPE",
        "_NET_WM_WINDOW_TYPE_MENU",
        "_NET_WM_WINDOW_TYPE_DIALOG",
        "_NET_WM_WINDOW_TYPE_DOCK",
        "_NET_WM_WINDOW_TYPE_POPUP_MENU",
        "_NET_WM_PID",
        "UTF8_STRING",
        "STRING",
        "COMPOUND_TEXT",
        atom_name,
    };
#define ATOMS_COUNT (sizeof(atom_names) / sizeof(char*))
    Atom atoms_return[ATOMS_COUNT];
    XInternAtoms(ui->dpy, atom_names, ATOMS_COUNT, False, atoms_return);
#undef ATOMS_COUNT

    ui->windowTypeAtom = atoms_return[0];
    ui->typeMenuAtom = atoms_return[1];
    ui->typeDialogAtom = atoms_return[2];
    ui->typeDockAtom = atoms_return[3];
    ui->typePopupMenuAtom = atoms_return[4];
    ui->pidAtom = atoms_return[5];
    ui->utf8Atom = atoms_return[6];
    ui->stringAtom = atoms_return[7];
    ui->compTextAtom = atoms_return[8];
    ui->compManagerAtom = atoms_return[9];
}
