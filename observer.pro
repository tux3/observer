#-------------------------------------------------
#
# Project created by QtCreator 2017-08-12T18:05:30
#
#-------------------------------------------------

QT       += core gui charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = observer
TEMPLATE = app

CONFIG += c++1z

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
    tabs/cpu/cputab.cpp \
    tabs/gpu/gputab.cpp \
    tabs/overview/overviewtab.cpp \
    tabs/procs/processtreewidgetitem.cpp \
    tabs/procs/procstab.cpp \
    tabs/procs/procstablecolumn.cpp \
    main.cpp \
    mainwindow.cpp \
    tabwidget.cpp

HEADERS += \
    tabs/cpu/cputab.h \
    tabs/gpu/gputab.h \
    tabs/overview/overviewtab.h \
    tabs/procs/processtreewidgetitem.h \
    tabs/procs/procstab.h \
    tabs/procs/procstablecolumn.h \
    mainwindow.h \
    tabwidget.h

FORMS += \
        mainwindow.ui \
        tabs/procs/procstab.ui \
        tabs/cpu/cputab.ui \
        tabs/gpu/gputab.ui \
        tabs/overview/overviewtab.ui

RESOURCES += \
    res.qrc

LIBS += -L/usr/lib/x86_64-linux-gnu/nvidia/current/ -lnvidia-ml
