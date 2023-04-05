// X11 实现的 TrayIcon
// 只有银河麒麟、中科方德 v10.1 使用，TrayIcon 不被收纳

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

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xmd.h>
#include <X11/extensions/Xrender.h>

#include "TrayWindow.h"
#include "tray.h"
//#include "skin.h"
//#include "classicui.h"
#include "fcitx-utils/log.h"
#include "fcitx/frontend.h"
#include "fcitx/module.h"
//#include "MenuWindow.h"
#include "fcitx/instance.h"
#include "fcitx-utils/utils.h"


void SetWindowProperty(UI* ui, Window window, FcitxXWindowType type, char* window_title);

void TrayWindowInit(TrayWindow *trayWindow)
{
    //FcitxClassicUI *classicui = trayWindow->owner;
    UI *ui = trayWindow->owner;
    Display *dpy = ui->dpy;
    int iScreen = ui->iScreen;
    char   strWindowName[] = "Fcitx Tray Window";

    if (trayWindow->window == None && trayWindow->dockWindow != None) {
        XVisualInfo* vi = TrayGetVisual(trayWindow);
        if (vi && vi->visual) {
            Window p = DefaultRootWindow(dpy);
            Colormap colormap = XCreateColormap(dpy, p, vi->visual, AllocNone);
            XSetWindowAttributes wsa;
            wsa.background_pixmap = 0;
            wsa.colormap = colormap;
            wsa.background_pixel = 0;
            wsa.border_pixel = 0;   
            trayWindow->window = XCreateWindow(dpy, p, -1, -1, 22, 22,
                                            0, vi->depth, InputOutput, vi->visual,
                                            CWBackPixmap | CWBackPixel | CWBorderPixel | CWColormap, &wsa);
        } else {
            trayWindow->window = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy),
                                -1, -1, 22, 22, 0,
                                BlackPixel(dpy, DefaultScreen(dpy)),
                                WhitePixel(dpy, DefaultScreen(dpy)));
            XSetWindowBackgroundPixmap(dpy, trayWindow->window, ParentRelative);
        }
        if (trayWindow->window == (Window) NULL)
            return;

        //Atom WM_DELETE_WINDOW = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
        //XSetWMProtocols(dpy, trayWindow->window, &WM_DELETE_WINDOW, 1);
        //XMapWindow(dpy, trayWindow->window);


        //XSetInputFocus(dpy, None, RevertToParent, CurrentTime);

        trayWindow->size = 22;
        XSizeHints size_hints;
        size_hints.flags = PWinGravity | PBaseSize;
        size_hints.base_width = trayWindow->size;
        size_hints.base_height = trayWindow->size;
        XSetWMNormalHints(dpy, trayWindow->window, &size_hints);

        if (vi && vi->visual)
        {
            trayWindow->cs_x = cairo_xlib_surface_create(dpy, trayWindow->window, trayWindow->visual.visual, 200, 200);
        }
        else {
            Visual *target_visual = DefaultVisual(dpy, iScreen);
            trayWindow->cs_x = cairo_xlib_surface_create(dpy, trayWindow->window, target_visual, 200, 200);
        }
        trayWindow->cs = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 200, 200);

        XSelectInput(dpy, trayWindow->window, ExposureMask | KeyPressMask |
                    ButtonPressMask | ButtonReleaseMask | StructureNotifyMask
                    | EnterWindowMask | PointerMotionMask | LeaveWindowMask | VisibilityChangeMask);

        //ClassicUISetWindowProperty(classicui, trayWindow->window, FCITX_WINDOW_DOCK, strWindowName);
        SetWindowProperty(ui, trayWindow->window, FCITX_WINDOW_DOCK, strWindowName);

        TrayFindDock(trayWindow);
    }
}

void TrayWindowCreate(UI *classicui, TrayWindow* trayWindow)
{
    trayWindow->visual.visual = NULL;
    trayWindow->owner = classicui;
    TrayInitAtom(trayWindow);
    trayWindow->dockWindow = TrayGetDock(trayWindow);
    //FcitxX11AddXEventHandler(classicui->owner, TrayEventHandler, trayWindow);
    /* We used to init trayWindow here, but now we should delay it */
}

void TrayWindowRelease(TrayWindow *trayWindow)
{
    UI *classicui = trayWindow->owner;
    Display *dpy = classicui->dpy;
    trayWindow->bTrayMapped = false;
    memset(&trayWindow->visual, 0, sizeof(trayWindow->visual));

    if (trayWindow->window == None)
        return;

    cairo_surface_destroy(trayWindow->cs);
    cairo_surface_destroy(trayWindow->cs_x);
    XDestroyWindow(dpy, trayWindow->window);
    trayWindow->window = None;
    trayWindow->cs = NULL;
    trayWindow->cs_x = NULL;
}

void TrayWindowDraw(TrayWindow* trayWindow)
{
    cairo_t *c;
    cairo_surface_t *png_surface;

    string path = string("/opt/apps/huayupy/files/skin/default/huayupy_tray.png");
    png_surface = LoadImage(path);
    if (png_surface == NULL)
        return;
    /* active, try draw im icon on tray */

    //png_surface = image->image;

    c = cairo_create(trayWindow->cs);
    cairo_set_source_rgba(c, 1, 1, 1, 0);
    cairo_set_operator(c, CAIRO_OPERATOR_SOURCE);
    cairo_paint(c);    

    do {
        if (png_surface) {
            int w = cairo_image_surface_get_width(png_surface);
            int h = cairo_image_surface_get_height(png_surface);
            if (w == 0 || h == 0)
                break;
            double scaleW = 1.0, scaleH = 1.0;
            if (w > trayWindow->size || h > trayWindow->size)
            {
                scaleW = ((double) trayWindow->size) / w;
                scaleH = ((double) trayWindow->size) / h;
                if (scaleW > scaleH)
                    scaleH = scaleW;
                else
                    scaleW = scaleH;
            }
            int aw = scaleW * w;
            int ah = scaleH * h;

            cairo_scale(c, scaleW, scaleH);
            cairo_set_source_surface(c, png_surface, (trayWindow->size - aw) / 2 , (trayWindow->size - ah) / 2);
            cairo_set_operator(c, CAIRO_OPERATOR_OVER);
            cairo_paint_with_alpha(c, 1);

            //cairo_surface_flush(trayWindow->cs);
        }
    } while(0);

    cairo_destroy(c);
    cairo_surface_destroy(png_surface);

    XVisualInfo* vi = trayWindow->visual.visual ? &trayWindow->visual : NULL;
    if (!(vi && vi->visual)) {
        XClearArea(trayWindow->owner->dpy, trayWindow->window, 0, 0, trayWindow->size, trayWindow->size, False);
    }

    c = cairo_create(trayWindow->cs_x);
    if (vi && vi->visual) {
        cairo_set_source_rgba(c, 0, 0, 0, 0);
        cairo_set_operator(c, CAIRO_OPERATOR_SOURCE);
        cairo_paint(c);
    }
    cairo_set_operator(c, CAIRO_OPERATOR_OVER);
    cairo_set_source_surface(c, trayWindow->cs, 0, 0);

    cairo_rectangle(c, 0, 0, trayWindow->size, trayWindow->size);
    //cairo_xlib_surface_set_size(trayWindow->cs_x, 22, 22);
    cairo_clip(c);
    cairo_paint(c);
    cairo_destroy(c);    
    cairo_surface_flush(trayWindow->cs_x);
}



void SetWindowProperty(UI* ui, Window window, FcitxXWindowType type, char* window_title)
{
    Atom *wintype = NULL;
    switch (type) {
    case FCITX_WINDOW_DIALOG:
        wintype = &ui->typeDialogAtom;
        break;
    case FCITX_WINDOW_DOCK:
        wintype = &ui->typeDockAtom;
        break;
    case FCITX_WINDOW_POPUP_MENU:
        wintype = &ui->typePopupMenuAtom;
        break;
    case FCITX_WINDOW_MENU:
        wintype = &ui->typeMenuAtom;
        break;
    default:
        wintype = NULL;
        break;
    }
    if (wintype)
        XChangeProperty(ui->dpy, window, ui->windowTypeAtom,
                        XA_ATOM, 32, PropModeReplace, (unsigned char*)wintype, 1);

    pid_t pid = getpid();
    XChangeProperty(ui->dpy, window, ui->pidAtom, XA_CARDINAL, 32,
                    PropModeReplace, (unsigned char*)&pid, 1);

    char res_name[] = "fcitx";
    char res_class[] = "fcitx";
    XClassHint ch;
    ch.res_name = res_name;
    ch.res_class = res_class;
    XSetClassHint(ui->dpy, window, &ch);

    if (window_title) {
        XTextProperty   tp;
        memset(&tp, 0, sizeof(XTextProperty));
        Xutf8TextListToTextProperty(ui->dpy, &window_title, 1,
                                    XUTF8StringStyle, &tp);
        if (tp.value) {
            XSetWMName(ui->dpy, window, &tp);
            XFree(tp.value);
        }
    }
}

cairo_surface_t* LoadImage(const std::string& file_path)
{
    FILE* fd = NULL;
    fd = fopen(file_path.c_str(), "rb");
    if (!fd)
        return NULL;
    fclose(fd);

    cairo_surface_t *png = NULL;
    png = cairo_image_surface_create_from_png(file_path.c_str());          
    if(!png)
        return NULL;

    if (cairo_surface_status (png)) {
        cairo_surface_destroy(png);
        png = NULL;
    }

    return png;
}

// kate: indent-mode cstyle; space-indent on; indent-width 0;
