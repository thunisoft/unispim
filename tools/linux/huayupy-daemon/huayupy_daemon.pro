QT += core dbus network

CONFIG += c++11

TARGET = huayupy-daemon-fcitx
#TARGET = huayupy-daemon-ibus
#CONFIG += console
CONFIG -= app_bundle


CONFIG += link_pkgconfig
# if bulild with ibus, define IBUS in CONFIG

#DEFINES += USE_IBUS

# if bulild with ibus, define IBUS in CONFIG
#CONFIG += IBUS

IBUS {
PKGCONFIG += ibus-1.0
} else {
}
DEFINES += USE_UOS

TEMPLATE = app

SOURCES += main.cpp \
    log.cc \
    configbus.cpp \
    statsinfouploader.cpp \
    uploadcache.cpp \
    inputmethodframeworkmonitor.cpp \
    maincontroller.cpp \
    wordlibdownloader.cpp \
    syswordlibdownloader.cpp \
    syscloudwordlibdownloader.cpp \
    userwordlibdownloader.cpp \
    wordlibcontroller.cpp \
    userwordlibuploader.cpp \
    dbusmsgmanager.cpp \
#    classifywordlibupdater.cpp \
    syswordlibupdatehelper.cpp \
    ../public/config.cpp \
    ../public/utils.cpp \
    ../public/zip.cpp \
    ../public/inisetting.cpp \
    ../public/dbusmessager.cpp \
    ../public/DateHelper.cpp \
    recordwrodlibuploader.cpp \
    updatechecker.cpp \
    networkhandler.cpp \
    professwordlibupdatetask.cpp \
    bakserverchecktask.cpp \
    syswordlibincretask.cpp \
    userwordlibupdatetask.cpp \
    ../public/configmanager.cpp \
    ../../../config/data/configitemint.cpp \
    ../../../config/data/configitemstr.cpp \
    ../../../config/data/data-decorator.cpp \
    ../../../config/data/datetime-decorator.cpp \
    ../../../config/data/entity.cpp \
    ../../../config/data/enumerator-decorator.cpp \
    ../../../config/data/int-decorator.cpp \
    ../../../config/data/string-decorator.cpp \
    ../../../config/data/systemconfig.cpp \
    ../../../config/data/userconfig.cpp


include(../qtsingleapplication/src/qtsingleapplication.pri)

DBUS_ADAPTORS += com.thunisoft.huayupy.xml

DBUS_INTERFACES += com.thunisoft.huayupy.xml

HEADERS += \
                log.h \
                configbus.h \
                configbusdbus.h \
                configbusibus.h \
    statsinfouploader.h \
    uploadcache.h \
    inputmethodframeworkmonitor.h \
    maincontroller.h \
    wordlibdownloader.h \
    syswordlibdownloader.h \
    syscloudwordlibdownloader.h \
    userwordlibdownloader.h \
    wordlibcontroller.h \
    userwordlibuploader.h \
    dbusmsgmanager.h \
#    classifywordlibupdater.h \
    syswordlibupdatehelper.h \
    ../public/utils.h \
    ../public/config.h \
    ../public/defines.h \
    ../public/miniz.h \
    ../public/zip.h \ 
    ../public/inisetting.h \
    ../public/dbusmessager.h \
    ../public/DateHelper.h \
    recordwrodlibuploader.h \
    updatechecker.h \
    networkhandler.h \
    professwordlibupdatetask.h \
    bakserverchecktask.h \
    syswordlibincretask.h \
    userwordlibupdatetask.h \
    ../public/configmanager.h \
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
    ../../../config/data/userconfig.h

RESOURCES += \
    cretificate.qrc

