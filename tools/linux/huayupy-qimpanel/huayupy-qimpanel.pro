#-------------------------------------------------
#
# Project created by QtCreator 2019-08-12T16:07:48
#
#-------------------------------------------------

DEFINES += QT5
DEFINES += USE_FCITX
#DEFINES += V10_1

QT      += core gui dbus svg xml
QMAKE_CFLAGS += -g -rdynamic
QMAKE_CXXFLAGS += -g -rdynamic -std=c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = huayupy-qimpanel
TEMPLATE = app
INCLUDEPATH += ../../fcitx/src/lib \


SOURCES += main.cpp\
    maincontroller.cpp \
    kimpanelagent.cpp \
    systemtraymenu.cpp \
    myaction.cpp \    
    skin/skinbase.cpp \
    skin/skinfcitx.cpp \
    mainmodel.cpp \
    candidateword.cpp \
    toolbarmodel.cpp \
    ../public/utils.cpp \
    ../public/config.cpp \
    ../public/configbus.cpp \
    ../public/zip.cpp \
    ../public/inisetting.cpp \
    ../public/dbusmessager.cpp \
    TrayWindow.cpp \
    traycontroller.cpp \
    tray.cpp \
    customize_qwidget.cpp \
    candidatewindow.cpp \
    windowconfig.cpp \
    toolbarwindow.cpp \
    svgutils.cpp \
    uosmodemonitor.cpp \
    tooltip.cpp \
    deletetipwidget.cpp \
    toolbarconfigwidget.cpp \
    toolbarmenuitem.cpp \
    toolbarmenu.cpp \
    ../public/addon_config.cpp \
    ../public/configmanager.cpp \
    ../../../config/data/userconfig.cpp \
    ../../../config/data/systemconfig.cpp \
    ../../../config/data/string-decorator.cpp \
    ../../../config/data/int-decorator.cpp \
    ../../../config/data/enumerator-decorator.cpp \
    ../../../config/data/entity.cpp \
    ../../../config/data/datetime-decorator.cpp \
    ../../../config/data/data-decorator.cpp \
    ../../../config/data/configitemstr.cpp \
    ../../../config/data/configitemint.cpp

HEADERS  += \
    maincontroller.h \
    kimpanelagent.h \
    agenttype.h \
    systemtraymenu.h \
    myaction.h \    
    skin/skinbase.h \
    skin/skinfcitx.h \
    mainmodel.h \
    candidateword.h \
    toolbarmodel.h \
    ../public/utils.h \
    ../public/config.h \
    ../public/configbus.h \
    ../public/configbusibus.h \
    ../public/configbusdbus.h \
    ../public/zip.h \
    ../public/miniz.h \
    ../public/inisetting.h \
    ../public/dbusmessager.h \
    traycontroller.h \
    TrayWindow.h \
    tray.h \
    customize_qwidget.h \
    candidatewindow.h \
    windowconfig.h \
    toolbarwindow.h \
    svgutils.h \
    uosmodemonitor.h \
    tooltip.h \
    deletetipwidget.h \
    toolbarconfigwidget.h \
    toolbarmenuitem.h \
    toolbarmenu.h \
    ../public/addon_config.h \
    ../public/configmanager.h \
    ../../../config/data/userconfig.h \
    ../../../config/data/systemconfig.h \
    ../../../config/data/string-decorator.h \
    ../../../config/data/int-decorator.h \
    ../../../config/data/enumerator-decorator.h \
    ../../../config/data/entity.h \
    ../../../config/data/entity-collection.h \
    ../../../config/data/datetime-decorator.h \
    ../../../config/data/data-decorator.h \
    ../../../config/data/configitemstr.h \
    ../../../config/data/configitemint.h

FORMS    += \
    deletetipwidget.ui \
    toolbarconfigwidget.ui \
    toolbarmenuitem.ui

DBUS_ADAPTORS += org.kde.impanel.xml

DBUS_INTERFACES += org.kde.impanel.xml

RESOURCES += \
    qml.qrc

include(../qtsingleapplication/src/qtsingleapplication.pri)

LIBS += -lX11  \
        -lXtst \
        -lcairo
