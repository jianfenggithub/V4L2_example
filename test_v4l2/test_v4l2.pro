#-------------------------------------------------
#
# Project created by QtCreator 2018-11-07T04:42:01
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = test_v4l2
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp \
    v4l2.cpp \
    camerathread.cpp

HEADERS  += widget.h \
    v4l2.h \
    camerathread.h

FORMS    += widget.ui
