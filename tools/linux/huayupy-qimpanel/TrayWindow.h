/***************************************************************************
 *   Copyright (C) 2002~2010 by Yuking                                     *
 *   yuking_net@sohu.com                                                   *
 *   Copyright (C) 2010~2010 by CSSlayer                                   *
 *   wengxt@gmail.com                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.              *
 ***************************************************************************/

#include "fcitx/fcitx.h"

#ifndef _TRAY_WINDOW_H
#define _TRAY_WINDOW_H

#include <X11/Xlib.h>
#include <string.h>
#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>
#include <X11/Xutil.h>
#include "fcitx-config/fcitx-config.h"
#include <X11/Xdefs.h>
#include <string>

#define INACTIVE_ICON 0
#define ACTIVE_ICON   1

using namespace std;

struct _FcitxClassicUI;

typedef void (*ShowMenuCallback)(void *main_controller_pointer, int x, int y);
typedef void (*ChangeIMCallback)(void *main_controller_pointer);


typedef struct _TrayWindow {
    Window window;
    boolean bTrayMapped;
    XVisualInfo visual;
    Atom atoms[6];

    cairo_surface_t *cs_x;
    cairo_surface_t *cs;
    int size;
    struct _UI* owner;
    Window dockWindow;
    ShowMenuCallback show_menu_callback;
    ChangeIMCallback change_im_callback;
    void* main_controller_pointer;
} TrayWindow;

typedef struct _UI {
    FcitxGenericConfig gconfig;
    Display* dpy;
    int iScreen;
    Atom protocolAtom;
    Atom killAtom;
    //struct _InputWindow* inputWindow;
    //struct _MainWindow* mainWindow;
    struct _TrayWindow* trayWindow;
    //FcitxUIMenu skinMenu;

    //FcitxSkin skin;
    UT_array skinBuf;
    //struct _FcitxInstance *owner;

    int fontSize;
    char* font;
    char* menuFont;
    char* strUserLocale;
    boolean bUseTrayIcon;
    boolean bUseTrayIcon_;
    //HIDE_MAINWINDOW hideMainWindow;
    boolean bVerticalList;
    char* skinType;
    int iMainWindowOffsetX;
    int iMainWindowOffsetY;

    UT_array status;
    //struct _XlibMenu* mainMenuWindow;
    //FcitxUIMenu mainMenu;
    boolean isSuspend;
    boolean isfallback;

    int dpi;
    boolean hasXShape;
    uint64_t trayTimeout;
    boolean notificationItemAvailable;

    unsigned int epoch;
    uint64_t waitDelayed;

    Atom typeMenuAtom;
    Atom windowTypeAtom;
    Atom typeDialogAtom;
    Atom typeDockAtom;
    Atom typePopupMenuAtom;
    Atom pidAtom;
    Atom utf8Atom;
    Atom stringAtom;
    Atom compTextAtom;
    Atom compManagerAtom;

    struct _FcitxSkin* skin;
} UI;

/**
 * Tray Icon Image
 **/
typedef struct _SkinTrayIcon {
    /**
     * Active Tray Icon Image
     **/
    char* active;

    /**
     * Inactive Tray Icon Image
     **/
    char* inactive;
} SkinTrayIcon;

typedef enum _FcitxXWindowType {
    FCITX_WINDOW_UNKNOWN,
    FCITX_WINDOW_DOCK,
    FCITX_WINDOW_POPUP_MENU,
    FCITX_WINDOW_MENU,
    FCITX_WINDOW_DIALOG
} FcitxXWindowType;

typedef struct _SkinInfo {
    char *skinName;
    char *skinVersion;
    char *skinAuthor;
    char *skinDesc;
} SkinInfo;

typedef enum _FillRule {
    F_COPY = 0,
    F_RESIZE = 1
} FillRule;

typedef enum _OverlayDock {
    OD_TopLeft = 0,
    OD_TopCenter = 1,
    OD_TopRight = 2,
    OD_CenterLeft = 3,
    OD_Center = 4,
    OD_CenterRight = 5,
    OD_BottomLeft = 6,
    OD_BottomCenter = 7,
    OD_BottomRight = 8,
} OverlayDock;

typedef struct _FcitxWindowBackground {
    char* background;

    char* overlay;
    OverlayDock dock;
    int overlayOffsetX;
    int overlayOffsetY;

    int marginTop;
    int marginBottom;
    int marginLeft;
    int marginRight;

    int clickMarginTop;
    int clickMarginBottom;
    int clickMarginLeft;
    int clickMarginRight;

    FillRule fillV;
    FillRule fillH;
} FcitxWindowBackground;

typedef struct _SkinMenu {
    FcitxWindowBackground background;
    FcitxConfigColor activeColor;
    FcitxConfigColor lineColor;
} SkinMenu;

typedef struct _SkinImage {
    char *name;
    cairo_surface_t *image;
    boolean textIcon;
    UT_hash_handle hh;
} SkinImage;

/**
* 配置文件结构,方便处理,结构固定
*/
typedef struct _FcitxSkin {
    SkinInfo skinInfo;
    SkinTrayIcon skinTrayIcon;
    SkinMenu skinMenu;    

    char** skinType;

    SkinImage* imageTable;
    SkinImage* trayImageTable;
} FcitxSkin;

void TrayWindowCreate(UI *classicui, TrayWindow* trayWindow);
void TrayWindowDraw(TrayWindow* trayWindow);
void TrayWindowRedraw(TrayWindow* trayWindow);
/* these two function can be called more than once */
void TrayWindowRelease(TrayWindow* trayWindow);
void TrayWindowInit(TrayWindow *trayWindow);

void SetWindowProperty(UI* ui, Window window, FcitxXWindowType type, char* window_title);
cairo_surface_t* LoadImage(const std::string& file_path);
#endif

// kate: indent-mode cstyle; space-indent on; indent-width 0;
