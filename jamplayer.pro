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
        player.cpp \
    music.cpp


HEADERS  += mainwindow.h \
        player.h \
    music.h


