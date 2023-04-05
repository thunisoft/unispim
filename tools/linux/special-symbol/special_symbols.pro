#-------------------------------------------------
#
# Project created by QtCreator 2019-09-17T15:32:34
#
#-------------------------------------------------

QT       += core gui dbus

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
#TARGET = special-symbols-ibus
TARGET = special-symbols-fcitx
TEMPLATE = app

CONFIG += link_pkgconfig
#CONFIG += no_keywords

#config += IBUS

IBUS {
PKGCONFIG += ibus-1.0
} else {
}

DEFINES += USE_UOS

SOURCES += main.cpp\
        mainwindow.cpp \
    customtabstyle.cpp \
    characterwidget.cpp \
    characterparser.cpp \
    tabwidget.cpp \
    charhandler.cpp \
    recentrecord.cpp \
    inputservice.cpp \
    config.cpp \
    customlabel.cpp

HEADERS  += mainwindow.h \
    customtabstyle.h \
    characterwidget.h \
    characterparser.h \
    tabwidget.h \
    charhandler.h \
    recentrecord.h \
    inputservice.h \
    config.h \
    customlabel.h

FORMS    += mainwindow.ui

RESOURCES += \
    symbol.qrc

include(../qtsingleapplication/src/qtsingleapplication.pri)

DBUS_ADAPTORS += com.thunisoft.huayupy.xml
DBUS_INTERFACES += com.thunisoft.huayupy.xml
