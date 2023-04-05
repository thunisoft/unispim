#-------------------------------------------------
#
# Project created by QtCreator 2022-02-23T13:31:07
#
#-------------------------------------------------

QT       += core gui network concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = toolBox
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

win32{
    include(qhotkey.pri)
}


RC_FILE += ./appicon.rc

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    toolbtn.cpp \
    utils.cpp \
    zlib/zip.cpp \
    networkhandler.cpp \
    config.cpp \
    mytableview.cpp \
    shortcutsettingdlg.cpp \
    customkeysequenceedit.cpp \
    custominfombox.cpp \
    sharedmemorycheckthread.cpp \
    hotkeymanager.cpp \
    pipemsgthread.cpp \
    toolupdatetask.cpp \
    customporgressbar.cpp \
    customtooltip.cpp

HEADERS += \
        mainwindow.h \
    toolbtn.h \
    utils.h \
    zlib/miniz.h \
    zlib/zip.h \
    networkhandler.h \
    config.h \
    mytableview.h \
    commondef.h \
    shortcutsettingdlg.h \
    customkeysequenceedit.h \
    custominfombox.h \
    sharedmemorycheckthread.h \
    hotkeymanager.h \
    pipemsgthread.h \
    toolupdatetask.h \
    customporgressbar.h \
    customtooltip.h

FORMS += \
        mainwindow.ui \
    shortcutsettingdlg.ui \
    custominfombox.ui \
    customporgressbar.ui \
    customtooltip.ui


DEFINES += ZLIB_WINAPI
DEFINES += _UNICODE

win32{
#Add windows depend lib
LIBS += User32.Lib
LIBS += Advapi32.Lib
LIBS += gdi32.lib
LIBS += Ole32.lib

}

RESOURCES += \
    image.qrc
