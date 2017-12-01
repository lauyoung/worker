QT += core network xml
QT -= gui

TARGET = workDemo
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

INCLUDEPATH +=                           \
    src/src/                             \
    src/src/control/                     \
    src/src/kits/                        \
    src/src/user/                        \
    src/src/net/                         \
    src/src/server/

SOURCES += src/src/main.cpp \
    src/src/control/controller.cpp \
    src/src/net/networkhelper.cpp \
    src/src/kits/utils.cpp \
    src/src/user/userdataitem.cpp \
    src/src/server/workerclient.cpp

HEADERS += \
    src/src/control/controller.h \
    src/src/net/networkhelper.h \
    src/src/kits/utils.h \
    src/src/user/userdataitem.h \
    src/src/server/workerclient.h

