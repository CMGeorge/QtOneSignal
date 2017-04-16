QT += core

INCLUDEPATH += $$PWD/src/

SOURCES += $$PWD/src/onesignal.cpp
HEADERS += $$PWD/src/onesignal.h



winrt{
warning ("WINRT")
}else: android{
    warning ("Android")
    QT += androidextras
    INCLUDEPATH += $$PWD/src/android/
    SOURCES += $$PWD/src/android/onesignal_private.cpp
    HEADERS += $$PWD/src/android/onesignal_private.h
#    DEFINES += $$PWD/src/android/src/com/onesignal/OneSignalPrivate.java

    QONESIGNAL_JAVASRC.path = /src/com/onesignal
    QONESIGNAL_JAVASRC.files += $$files($$PWD/src/android/src/com/onesignal/*)

    INSTALLS += QONESIGNAL_JAVASRC
}else{
INCLUDEPATH += $$PWD/src/dummy/
    SOURCES += $$PWD/src/dummy/onesignal_private.cpp
    HEADERS += $$PWD/src/dummy/onesignal_private.h
}

warning($$INCLUDEPATH)
