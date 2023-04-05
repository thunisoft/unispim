#-------------------------------------------------
#
# Project created by QtCreator 2019-09-17T15:32:34
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = special_symbols
TEMPLATE = app

CONFIG += c++11

RC_FILE += ./appicon.rc

SOURCES += main.cpp\
    config.cpp \
    mainsymbolstackedwidget.cpp \
        mainwindow.cpp \
    customtabstyle.cpp \
    characterwidget.cpp \
    characterparser.cpp \
    searchfailed.cpp \
    searchlineedit.cpp \
    searchresultwidget.cpp \
    sharedmemorycheckthread.cpp \
    tabwidget.cpp \
    charhandler.cpp \
    recentrecord.cpp \
    inputservice.cpp

HEADERS  += mainwindow.h \
    config.h \
    customtabstyle.h \
    characterwidget.h \
    characterparser.h \
    mainsymbolstackedwidget.h \
    searchfailed.h \
    searchlineedit.h \
    searchresultwidget.h \
    sharedmemorycheckthread.h \
    tabwidget.h \
    charhandler.h \
    recentrecord.h \
    inputservice.h

FORMS += mainwindow.ui \
    mainsymbolstackedwidget.ui \
    searchfailed.ui \
    searchresultwidget.ui

INCLUDEPATH += "C:/Program Files (x86)/Windows Kits/10/Include/10.0.10240.0/ucrt"
LIBS += -L"C:/Program Files (x86)/Windows Kits/10/Lib/10.0.10240.0/ucrt/x86"

INCLUDEPATH += "C:/Program Files (x86)/Windows Kits/10/Include/10.0.10240.0/um"
LIBS += -L"C:/Program Files (x86)/Windows Kits/10/Lib/10.0.10240.0/um/x86"

INCLUDEPATH += "C:/Program Files (x86)/Windows Kits/10/Include/10.0.10240.0/shared"

LIBS += -L"C:/Program Files (x86)/Microsoft Visual Studio 14.0/VC/lib"
LIBS += User32.Lib
LIBS += gdi32.lib

win32: QMAKE_CXXFLAGS_RELEASE -= -Zc:strictStrings
win32: QMAKE_CFLAGS_RELEASE -= -Zc:strictStrings
win32: QMAKE_CFLAGS -= -Zc:strictStrings
win32: QMAKE_CXXFLAGS -= -Zc:strictStrings


RESOURCES += \
    style.qrc
