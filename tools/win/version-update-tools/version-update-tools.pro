QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

RC_FILE += AppIcon.rc
OTHER_FILES += \
    AppIcon.rc
SOURCES += \
    customize_qwidget.cpp \
    customizemessgebox.cpp \
    downloaddlg.cpp \
    main.cpp \
    packagedownloader.cpp \
    updatecontrol.cpp \
    updatewidget.cpp \
    utils.cpp

HEADERS += \
    customize_qwidget.h \
    customizemessgebox.h \
    downloaddlg.h \
    packagedownloader.h \
    updatecontrol.h \
    updatewidget.h \
    utils.h

FORMS += \
    customizemessgebox.ui \
    downloaddlg.ui \
    updatewidget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource.qrc

# https depends on openssl
LIBS += -L"../../../3rd/OpenSSL-Win32/lib" -llibeay32
LIBS += -L"../../../3rd/OpenSSL-Win32/lib" -lssleay32
INCLUDEPATH += -L"../../../3rd/OpenSSL-Win32/include"

INCLUDEPATH += "C:/Program Files (x86)/Windows Kits/10/Include/10.0.17763.0/ucrt"
LIBS += -L"C:/Program Files (x86)/Windows Kits/10/Lib/10.0.17763.0/ucrt/x86"

INCLUDEPATH += "C:/Program Files (x86)/Windows Kits/10/Include/10.0.17763.0/um"
LIBS += -L"C:/Program Files (x86)/Windows Kits/10/Lib/10.0.17763.0/um/x86"

INCLUDEPATH += "C:/Program Files (x86)/Windows Kits/10/Include/10.0.17763.0/shared"

#LIBS += -L"C:/Program Files (x86)/Microsoft Visual Studio 14.0/VC/lib"

LIBS += Advapi32.lib
LIBS += User32.Lib


win32: QMAKE_CXXFLAGS_RELEASE -= -Zc:strictStrings
win32: QMAKE_CFLAGS_RELEASE -= -Zc:strictStrings
win32: QMAKE_CFLAGS -= -Zc:strictStrings
win32: QMAKE_CXXFLAGS -= -Zc:strictStrings
