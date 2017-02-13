#-------------------------------------------------
#
# Project created by QtCreator 2017-01-28T20:43:17
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = jamplayer 
TEMPLATE = app
PKGCONFIG += alsa drumstick-alsa
CONFIG += link_pkgconfig

INCLUDEPATH += /usr/include/Qt5GStreamer/

LIBS += -lQt5GStreamer-1.0 -lQt5GStreamerUi-1.0 -lQt5GLib-2.0

SOURCES += main.cpp\
        mainwindow.cpp\
        sequenceradapter.cpp \
        player.cpp


HEADERS  += mainwindow.h \
        sequenceradapter.h \
        player.h

FORMS    += mainwindow.ui
