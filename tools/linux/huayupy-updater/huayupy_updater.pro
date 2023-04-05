#-------------------------------------------------
#
# Project created by QtCreator 2019-06-14T16:34:52
#
#-------------------------------------------------

QT       += core gui network dbus

CONFIG += c++11
#CONFIG += console
CONFIG -= app_bundle

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = huayupy-updater-fcitx
#TARGET = huayupy-updater-ibus

TEMPLATE = app

DEFINES += USE_UOS

SOURCES += main.cpp\
        mainwindow.cpp \
        checkupdateunispy.cpp \
        cJSON.c \
        iniparser.c \
        dictionary.c \
        updatewidget.cpp \
        tabwidgetbase.cpp \                
        updatetip.cpp \
        passworddialog.cpp \
        msgtodaemon.cpp \
        ../public/config.cpp \
        ../public/inisetting.cpp \
        ../public/dbusmessager.cpp \
        ../public/configbus.cpp \
    ../public/utils.cpp \
    ../public/zip.cpp \
    ../../../config/data/configitemint.cpp \
    ../../../config/data/configitemstr.cpp \
    ../../../config/data/data-decorator.cpp \
    ../../../config/data/datetime-decorator.cpp \
    ../../../config/data/entity.cpp \
    ../../../config/data/enumerator-decorator.cpp \
    ../../../config/data/int-decorator.cpp \
    ../../../config/data/string-decorator.cpp \
    ../../../config/data/systemconfig.cpp \
    ../../../config/data/userconfig.cpp \
    ../public/configmanager.cpp

HEADERS  += mainwindow.h \
            checkupdateunispy.h \
            cJSON.h \
            iniparser.h \
            dictionary.h \
            updatewidget.h \
            tabwidgetbase.h \
            define.h \
            commondef.h \
            updatetip.h \
            passworddialog.h \
            msgtodaemon.h \
            ../public/config.h \
            ../public/inisetting.h \
            ../public/dbusmessager.h \
            ../public/configbus.h \
            ../public/configbusibus.h \
            ../public/conifgbusdbus.h \
    ../public/utils.h \
    ../public/zip.h \
    ../public/miniz.h \
    ../../../config/data/configitemint.h \
    ../../../config/data/configitemstr.h \
    ../../../config/data/data-decorator.h \
    ../../../config/data/datetime-decorator.h \
    ../../../config/data/entity-collection.h \
    ../../../config/data/entity.h \
    ../../../config/data/enumerator-decorator.h \
    ../../../config/data/int-decorator.h \
    ../../../config/data/string-decorator.h \
    ../../../config/data/systemconfig.h \
    ../../../config/data/userconfig.h \
    ../public/configmanager.h

DBUS_ADAPTORS += com.thunisoft.update.xml
DBUS_INTERFACES += com.thunisoft.update.xml

FORMS    += mainwindow.ui \
    updatetip.ui \
    updatewidget.ui \
    password_dialog.ui

RESOURCES += \
    server_cer.qrc

RC_FILE += appicon.rc
