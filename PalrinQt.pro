#-------------------------------------------------
#
# Project created by QtCreator 2014-01-18T14:59:14
#
#-------------------------------------------------

QT       += core network script

#QT       -= gui

TARGET = PalrinQt
#CONFIG   += console
#CONFIG   -= app_bundle

TEMPLATE = lib

DEFINES += PALRINQT_LIBRARY

win32: LIBS += -lcryptlib
!win32: LIBS += -lcryptopp

SOURCES += \
    ConnectionManager.cpp \
    PalPacket.cpp \
    CryptoManager.cpp \
    DataMapParser.cpp \
    json.cpp \
    Utils.cpp \
    PacketBuilder.cpp \
    BaseClient.cpp \
    PalMessage.cpp

HEADERS += \
    ConnectionManager.h \
    PalPacket.h \
    CryptoManager.h \
    palrinqt_global.h \
    DataMapParser.h \
    json.h \
    Utils.h \
    PacketBuilder.h \
    BaseClient.h \
    PalMessage.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
win32 {
    target.path = $$[QT_INSTALL_PREFIX]/lib
    INSTALLS += target
}

RESOURCES += \
    resources.qrc
