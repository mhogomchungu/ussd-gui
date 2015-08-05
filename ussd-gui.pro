#-------------------------------------------------
#
# Project created by QtCreator 2015-07-26T15:42:43
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ussd-gui
TEMPLATE = app


SOURCES += src/main.cpp src/mainwindow.cpp 3rd_party/qgsmcodec.cpp \
    src/gsm.cpp

HEADERS  += src/mainwindow.h src/task.h 3rd_party/qgsmcodec.h \
    src/gsm.h

FORMS    += src/mainwindow.ui

QMAKE_CXXFLAGS += -std=c++11

LIBS += -lGammu

INCLUDEPATH += /usr/include/gammu

RESOURCES   += /home/mtz/projects/ussd-gui/icons/icons.qrc
INCLUDEPATH +=/home/mtz/projects/build/ussd
