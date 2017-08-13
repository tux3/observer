#-------------------------------------------------
#
# Project created by QtCreator 2017-08-12T18:05:30
#
#-------------------------------------------------

QT       += core gui

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
        main.cpp \
        mainwindow.cpp \
        tabwidget.cpp \
        tabs/procs/procstab.cpp \
        tabs/procs/processtreewidgetitem.cpp \
        tabs/cpu/cputab.cpp \
    tabs/procs/procstablecolumn.cpp

HEADERS += \
        mainwindow.h \
        tabwidget.h \
        tabs/procs/procstab.h \
        tabs/procs/processtreewidgetitem.h \
        tabs/cpu/cputab.h \
    tabs/procs/procstablecolumn.h

FORMS += \
        mainwindow.ui \
        tabs/procs/procstab.ui \
        tabs/cpu/cputab.ui

RESOURCES += \
    res.qrc
