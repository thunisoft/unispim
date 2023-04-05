#-------------------------------------------------
#
# Project created by QtCreator 2019-03-14T15:09:27
#
#-------------------------------------------------

DEFINES += QT5
QT       += core gui dbus network svg xml concurrent#charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

#TARGET = huayupy-config-tools-ibus
TARGET = huayupy-config-tools-fcitx
TEMPLATE = app

CONFIG += link_pkgconfig

# if bulild with ibus, define IBUS in CONFIG
#CONFIG += IBUS
IBUS {
PKGCONFIG += ibus-1.0
} else {
}

DEFINES += USE_FCITX

DEFINES += CONFIG_TOOL


#INCLUDEPATH += /usr/include/x86_64-linux-gnu/qt5/QtGui/#

SOURCES += main.cpp\
        mainwindow.cpp \
    configbus.cpp \
    cJSON.c \
    dictionary.c \
    iniparser.c \
    suggestsetwidget.cpp \
    checkupdatefaild.cpp \    
    phonecorrectdlg.cpp \
    customize_ui/customize_qwidget.cpp \
    customize_ui/customizecombobox.cpp \
    customize_ui/customizeslider.cpp \
    customize_ui/customizefont.cpp \
    customize_ui/customizetablewidgetforaddwordlib.cpp \
    customize_ui/customize_mainwindow.cpp \
    toviewphrasedialog.cpp \    
    phraseoptionwidget.cpp \
    addoreditphrasedialog.cpp \
    customize_ui/customizeqtextedit.cpp \
    fileviewerdialog.cpp \
    aboutstackedwidget.cpp\
    phrasevalidator.cpp \
    accountstackedwidget.cpp \
    fuzzydialog.cpp \
    wordlibmainwidget.cpp \
    wordlibshowwidget.cpp \
    wordlibcellwidget.cpp \
    checkfailedstackedwidget.cpp \
    wordlibnetworker.cpp \
    registerstackedwidget.cpp \
    enrollstackedwidget.cpp \
    wordlibaddwidget.cpp \
    phrasemodel.cpp \
    editphrasefiledialog.cpp \
    customize_ui/customizemessageboxdialog.cpp \
    customize_ui/customizemessageboxwithreturnvaluedialog.cpp \
    customize_ui/generalstyledefiner.cpp \
    resetpasswdstackedwidget.cpp \
    feedback/feedbackstackedwidget.cpp \
    customize_ui/customtextedit.cpp \
    ../public/inisetting.cpp \
    ../public/utils.cpp \
    ../public/config.cpp \
    ../public/dbusmessager.cpp \
    ../public/zip.cpp \
    msgboxutils.cpp \
    wubistackedwidght.cpp \
    customize_ui/spkeyboard.cpp \
    customize_ui/singlekey.cpp \
    spschememodel.cpp \
    spschemeviewdlg.cpp \
    newschemedlg.cpp \
    newversionchecker.cpp \
    passworddialog.cpp \
    updatewidget.cpp \
    verififycodethread.cpp \
    skinstackedwidget.cpp \
    stylepreviewwidght.cpp \
    basicconfigstackedwidget.cpp \
    advancedconfigstackedwidget.cpp \
    customize_ui/stylesheetablewidget.cpp \
    editspfiledlg.cpp \
    networkhandler.cpp \
    importwordlibtemplatedlg.cpp \
    ../public/addon_config.cpp \
    wordlibpage/wordlibstackedwidget.cpp \
    customize_ui/customizebtngroup.cpp \
    wordlibpage/outputwordlibdlg.cpp \
    customize_ui/wordlibitemwidget.cpp \
    wordlibpage/informmsgbox.cpp \
    wordlibpage/mywordlibtableview.cpp \
    wordlibpage/syswordlibcheckthread.cpp \
    wordlibpage/updatesyswordlibthread.cpp \
    wordlibpage/adduserwordlibtableitem.cpp \
    wordlibpage/wordlibtableitemwidget.cpp \
    wordlibpage/myutils.cpp \
    ../../../config/data/userconfig.cpp \
    ../../../config/data/systemconfig.cpp \
    ../../../config/data/string-decorator.cpp \
    ../../../config/data/int-decorator.cpp \
    ../../../config/data/enumerator-decorator.cpp \
    ../../../config/data/entity.cpp \
    ../../../config/data/datetime-decorator.cpp \
    ../../../config/data/data-decorator.cpp \
    ../../../config/data/configitemstr.cpp \
    ../../../config/data/configitemint.cpp \
    ../public/configmanager.cpp \
    personpage/personaccountstackedwidget.cpp

HEADERS  += mainwindow.h \
    configbus.h \
    configbusdbus.h \  
    cJSON.h \
    dictionary.h \
    iniparser.h \    
    suggestsetwidget.h \
    checkupdatefaild.h \
    phonecorrectdlg.h \
    customize_ui/customize_qwidget.h \
    customize_ui/customizecombobox.h \
    customize_ui/customizeslider.h \
    customize_ui/customizefont.h \
    customize_ui/customizetablewidgetforaddwordlib.h \
    customize_ui/customize_mainwindow.h \
    toviewphrasedialog.h \
    phraseoptionwidget.h \
    addoreditphrasedialog.h \
    customize_ui/customizeqtextedit.h \
    fileviewerdialog.h \
    aboutstackedwidget.h \
    phrasevalidator.h \
    accountstackedwidget.h \
    fuzzydialog.h \
    wordlibmainwidget.h \
    wordlibshowwidget.h \
    wordlibcellwidget.h \
    checkfailedstackedwidget.h \
    wordlibnetworker.h \
    registerstackedwidget.h \
    enrollstackedwidget.h \
    wordlibaddwidget.h \
    phrasemodel.h \
    editphrasefiledialog.h \
    customize_ui/customizemessageboxdialog.h \
    customize_ui/customizemessageboxwithreturnvaluedialog.h \
    customize_ui/generalstyledefiner.h \
    resetpasswdstackedwidget.h \
    feedback/feedbackstackedwidget.h \
    customize_ui/customtextedit.h \
    msgboxutils.h \
    ../public/inisetting.h \
    ../public/defines.h \
    ../public/utils.h \
    ../public/config.h \
    ../public/dbusmessager.h \
    ../public/miniz.h \
    ../public/zip.h \
    wubistackedwidght.h \
    customize_ui/spkeyboard.h \
    customize_ui/singlekey.h \
    spschememodel.h \
    spschemeviewdlg.h \
    newschemedlg.h \
    newversionchecker.h \
    passworddialog.h \
    updatewidget.h \
    verififycodethread.h \
    skinstackedwidget.h \
    stylepreviewwidght.h \
    basicconfigstackedwidget.h \
    advancedconfigstackedwidget.h \
    customize_ui/stylesheetablewidget.h \
    editspfiledlg.h \
    networkhandler.h \
    commondef.h \
    unispydef.h \
    importwordlibtemplatedlg.h \
    ../public/addon_config.h \
    wordlibpage/wordlibstackedwidget.h \
    customize_ui/customizebtngroup.h \
    wordlibpage/outputwordlibdlg.h \
    customize_ui/wordlibitemwidget.h \
    wordlibpage/informmsgbox.h \
    wordlibpage/mywordlibtableview.h \
    wordlibpage/syswordlibcheckthread.h \
    wordlibpage/updatesyswordlibthread.h \
    wordlibpage/adduserwordlibtableitem.h \
    wordlibpage/wordlibtableitemwidget.h \
    wordlibpage/myutils.h \
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
    ../../../config/data/configitemint.h \
    ../public/configmanager.h \
    personpage/personaccountstackedwidget.h

    DBUS_ADAPTORS += com.thunisoft.update.xml \
                     com.thunisoft.huayupy.xml
    DBUS_INTERFACES += com.thunisoft.update.xml \
                       com.thunisoft.huayupy.xml

FORMS    += \
            openofficewebsite.ui \
            mainwindow.ui \
    suggestset.ui \
    phrase.ui \
    editphrasedialog.ui \
    phonecorrectdlg.ui \
    toviewphrasedialog.ui \
    phraseoptionwidget.ui \
    addoreditphrasedialog.ui \
    fileviewerdialog.ui \
    aboutstackedwidget.ui \
    accountstackedwidget.ui \    
    fuzzydialog.ui \
    wordlibmainwidget.ui \
    wordlibshowwidget.ui \
    wordlibcellwidget.ui \
    checkfailedstackedwidget.ui \
    registerstackedwidget.ui \
    resetpasswdstackedwidget.ui \
    enrollstackedwidget.ui \
    wordlibaddwidget.ui \
    customize_ui/customizemessagebox.ui \
    editphrasefiledialog.ui \
    customize_ui/customizemessageboxdialog.ui \
    customize_ui/customizemessageboxwithreturnvaluedialog.ui \
    feedback/feedbackstackedwidget.ui \
    wubistackedwidght.ui \
    customize_ui/singlekey.ui \
    spschemeviewdlg.ui \
    newschemedlg.ui \
    password_dialog.ui \
    updatewidget.ui \
    basicconfigstackedwidget.ui \
    advancedconfigstackedwidget.ui \
    skinconfigstackedwidget.ui \
    editspfile.ui \
    importwordlibtemplatedlg.ui \
    wordlibpage/wordlibstackedwidget.ui \
    customize_ui/customizebtngroup.ui \
    wordlibpage/outputwordlibdlg.ui \
    customize_ui/wordlibitemwidget.ui \
    wordlibpage/informmsgbox.ui \
    wordlibpage/adduserwordlibtableitem.ui \
    wordlibpage/wordlibtableitemwidget.ui \
    personpage/personaccountstackedwidget.ui

include(../qtsingleapplication/src/qtsingleapplication.pri)

RESOURCES += \
    source.qrc

RC_FILE += appicon.rc

