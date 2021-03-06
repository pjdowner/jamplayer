QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = jamplayer 
TEMPLATE = app
PKGCONFIG += alsa drumstick-alsa
CONFIG += link_pkgconfig

INCLUDEPATH += /usr/include/Qt5GStreamer/
INCLUDEPATH += /usr/include/drumstick/

LIBS += -lQt5GStreamer-1.0 -lQt5GStreamerUi-1.0 -lQt5GLib-2.0

SOURCES += main.cpp\
        mainwindow.cpp\
        player.cpp \
    music.cpp \
    midicontrol.cpp


HEADERS  += mainwindow.h \
        player.h \
    music.h \
    midicontrol.h


