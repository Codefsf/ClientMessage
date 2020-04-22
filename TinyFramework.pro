#-------------------------------------------------
#
# Project created by QtCreator 2020-04-22T20:08:31
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TinyFramework
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    default_message_pump.cpp \
    FrameworkThread.cpp \
    lock_win.cpp \
    main.cpp \
    mainwindow.cpp \
    MessageLoop.cpp \
    MessageLoopProxy.cpp \
    MessagePump.cpp \
    Thread.cpp \
    ThreadLocal.cpp \
    ThreadManage.cpp \
    waitable_event_win.cpp

HEADERS += \
        mainwindow.h \
    base_config.h \
    base_types.h \
    Callback.hpp \
    default_message_pump.h \
    FrameworkThread.h \
    lock.h \
    mainwindow.h \
    MessageLoop.h \
    MessageLoopProxy.h \
    MessagePump.h \
    port.h \
    Thread.h \
    ThreadLocal.h \
    ThreadManage.h \
    waitable_event.h

FORMS += \
        mainwindow.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DEFINES += _X86_
