#-------------------------------------------------
#
# Project created by QtCreator 2015-07-26T15:42:43
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ussd-gui
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h task.h

FORMS    += mainwindow.ui

QMAKE_CXXFLAGS += -std=c++11

LIBS += -lGammu

INCLUDEPATH += /usr/include/gammu

RESOURCES += /home/mtz/projects/ussd-gui/icon.qrc
