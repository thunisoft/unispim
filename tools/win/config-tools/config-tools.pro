QT       += core gui network sql svg xml concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# VERSION = 1.6.77
# QMAKE_TARGET_DESCRIPTION = HuayuPY-Config

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
#DEFINES += BUILD_FY_VERSION

QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO




# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# RC_ICONS = logo.ico
RC_FILE += ./appicon.rc

TRANSLATIONS += widgets.ts

SOURCES += \
    aboutstackedwidget.cpp \
    addoreditphrasedialog.cpp \
    advancedconfigstackedwidget.cpp \
    basicconfigstackedwidget.cpp \
    checkfailedstackedwidget.cpp \
    config.cpp \
    customize_ui/basestackedwidget.cpp \
    customize_ui/customize_mainwindow.cpp \
    customize_ui/customize_qwidget.cpp \
    customize_ui/customize_tablewidget.cpp \
    customize_ui/customizebtngroup.cpp \
    customize_ui/customizephrasebtn.cpp \
    customize_ui/customtextedit.cpp \
    customize_ui/singlekey.cpp \
    customize_ui/spkeyboard.cpp \
    customize_ui/tableviewdelegate.cpp \
    customize_ui/wordlibitemwidget.cpp \
    downloadthread.cpp \
    dpiadaptor.cpp \
    editphrasefiledialog.cpp \
    element_style.cpp \
    feedbackstackedwidget.cpp \
    fileviewerdialog.cpp \
    fuzzydialog.cpp \
    informmsgbox.cpp \
    iniutils.cpp \
    main.cpp \
    mainwindow.cpp \
    mysqlite.cpp \
    networkhandler.cpp \
    networktest.cpp \
    newenrollstackedwidget.cpp \
    newschemedlg.cpp \
    outputwordlibdlg.cpp \
    personaccountstackedwidget.cpp \
    phrasemodel.cpp \
    phraseoptionwidget.cpp \
    phrasevalidator.cpp \
    servicecontroller.cpp \
    share_segment.c \
    sharedmemorycheckthread.cpp \
    skinconfigstackedwidget.cpp \
    spschememodel.cpp \
    spschemeviewdlg.cpp \
    spsetplandlg.cpp \
    stylepreviewwidght.cpp \
    suggestsetwidget.cpp \
    syllableapi.cpp \
    syswordlibcheckthread.cpp \
    toviewphrasedialog.cpp \
    updatesyswordlibthread.cpp \
    utils.cpp \
    wordlibapi.cpp \
    wordlibstackedwidget.cpp \
    wordlibutils.cpp \
    zlib/zip.cpp \
    tinyxml2.cpp \
    WLConverter.cpp \
    mywordlibtableview.cpp \
    wordlibtableitemwidget.cpp \
    adduserwordlibtableitem.cpp \
    ../../../toolBox/addon_config.cpp

HEADERS += \
    aboutstackedwidget.h \
    addoreditphrasedialog.h \
    advancedconfigstackedwidget.h \
    basicconfigstackedwidget.h \
    checkfailedstackedwidget.h \
    commondef.h \
    config.h \
    customize_ui/basestackedwidget.h \
    customize_ui/customize_mainwindow.h \
    customize_ui/customize_qwidget.h \
    customize_ui/customize_tablewidget.h \
    customize_ui/customizebtngroup.h \
    customize_ui/customizephrasebtn.h \
    customize_ui/customtextedit.h \
    customize_ui/singlekey.h \
    customize_ui/spkeyboard.h \
    customize_ui/tableviewdelegate.h \
    customize_ui/wordlibitemwidget.h \
    downloadthread.h \
    dpiadaptor.h \
    editphrasefiledialog.h \
    element_style.h \
    feedbackstackedwidget.h \
    fileviewerdialog.h \
    fuzzydialog.h \
    informmsgbox.h \
    iniutils.h \
    mainwindow.h \
    mysqlite.h \
    networkhandler.h \
    networktest.h \
    newenrollstackedwidget.h \
    newschemedlg.h \
    outputwordlibdlg.h \
    personaccountstackedwidget.h \
    phrasemodel.h \
    phraseoptionwidget.h \
    phrasevalidator.h \
    servicecontroller.h \
    share_segment.h \
    sharedmemorycheckthread.h \
    skinconfigstackedwidget.h \
    spschememodel.h \
    spschemeviewdlg.h \
    spsetplandlg.h \
    stylepreviewwidght.h \
    suggestsetwidget.h \
    syllableapi.h \
    syswordlibcheckthread.h \
    toviewphrasedialog.h \
    unistd.h \
    updatesyswordlibthread.h \
    utils.h \
    version.h \
    wordlibapi.h \
    wordlibdef.h \
    wordlibstackedwidget.h \
    wordlibutils.h \ \
    zlib/miniz.h \
    zlib/zip.h \
    tinyxml2.h \
    WLConverter.h \
    mywordlibtableview.h \
    wordlibtableitemwidget.h \
    adduserwordlibtableitem.h \
    ../../../toolBox/commondef.h \
    ../../../toolBox/addon_config.h

FORMS += \
    aboutstackedwidget.ui \
    addoreditphrasedialog.ui \
    advancedconfigstackedwidget.ui \
    basicconfigstackedwidget.ui \
    checkfailedstackedwidget.ui \
    customize_ui/customizebtngroup.ui \
    customize_ui/customizephrasebtn.ui \
    customize_ui/singlekey.ui \
    customize_ui/wordlibitemwidget.ui \
    editphrasefiledialog.ui \
    feedbackstackedwidget.ui \
    fileviewerdialog.ui \
    fuzzydialog.ui \
    informmsgbox.ui \
    mainwindow.ui \
    newenrollstackedwidget.ui \
    newschemedlg.ui \
    outputwordlibdlg.ui \
    personaccountstackedwidget.ui \
    phraseoptionwidget.ui \
    skinconfigstackedwidget.ui \
    spschemeviewdlg.ui \
    spsetplandlg.ui \
    suggestset.ui \
    toviewphrasedialog.ui \
    wordlibstackedwidget.ui \
    wordlibtableitemwidget.ui \
    adduserwordlibtableitem.ui

msvc{
    QMAKE_CFLAGS += /utf-8
    QMAKE_CXXFLAGS += /utf-8
}

INCLUDEPATH += "C:/Program Files (x86)/Windows Kits/10/Include/10.0.10240.0/ucrt"
LIBS += -L"C:/Program Files (x86)/Windows Kits/10/Lib/10.0.10240.0/ucrt/x86"

INCLUDEPATH += "C:/Program Files (x86)/Windows Kits/10/Include/10.0.10240.0/um"
LIBS += -L"C:/Program Files (x86)/Windows Kits/10/Lib/10.0.10240.0/um/x86"

INCLUDEPATH += "C:/Program Files (x86)/Windows Kits/10/Include/10.0.10240.0/shared"

LIBS += -L"C:/Program Files (x86)/Microsoft Visual Studio 14.0/VC/lib"

LIBS += -L"C:/Program Files (x86)/Windows Kits/10/Lib/10.0.17763.0/um/x86/Gdi32.Lib"



win32: QMAKE_CXXFLAGS_RELEASE -= -Zc:strictStrings
win32: QMAKE_CFLAGS_RELEASE -= -Zc:strictStrings
win32: QMAKE_CFLAGS -= -Zc:strictStrings
win32: QMAKE_CXXFLAGS -= -Zc:strictStrings

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    image.qrc

LIBS += User32.Lib
LIBS += Advapi32.Lib
LIBS += gdi32.lib



# https depends on openssl
LIBS += -L"../../../3rd/OpenSSL-Win32/lib" -llibeay32
LIBS += -L"../../../3rd/OpenSSL-Win32/lib" -lssleay32
INCLUDEPATH += "../../../3rd/OpenSSL-Win32/include"

DEFINES += _UNICODE

DEFINES += ZLIB_WINAPI


