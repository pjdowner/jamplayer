/*
Copyright (C) 2017 Philip Downer <phil@pjd.me.uk>
Copyright (C) 2010 Marco Ballesio <gibrovacco@gmail.com>
Copyright (C) 2011 Collabora Ltd.
 @author George Kiagiadakis <george.kiagiadakis@collabora.co.uk>

This library is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published
by the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "midicontrol.h"
#include "player.h"
#include "music.h"
#include <QMainWindow>
#include <QTimer>
#include <QWidget>
#include <QStyle>
#include <QObject>
#include <QSlider>
#include <QLabel>
#include <QListWidget>

class QBoxLayout;
class QToolButton;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QListWidget *songList;
    QListWidget *loopList;

private Q_SLOTS:
    void open();
    void loadFile();
    void openFile(const QString & fileName, const float pitch, const float tempo);
    void onStateChanged();
    void onPositionChanged();
    void pitchDown();
    void pitchUp();
    void slowDown();
    void speedUp();
    void keyed(int k);
    void play(void);
    void onDurationChanged();
    void getTimeStart();
    void getTimeStop();
    void timestop_back();
    void timestop_forward();
    void timestart_back();
    void timestart_forward();
    void timechange(QTime *time, QLabel *lbl, bool forward);

private:
    Player *jam_player;
    Music *MD;
    QToolButton *initButton(QStyle::StandardPixmap icon, const QString & tip,
                                      QObject *dstobj, const char *slot_method, QLayout *layout);

    midicontrol* mControl;

    void getTime(bool start);

    void createUI(QBoxLayout *appLayout);

    bool loadData();
    void readJson(QJsonObject &json);
    void updateList(QJsonArray ja);
    void updateLoopList(QJsonArray loops);

    void updatePitchLabel(float pitch);
    void updateTempoLabel(float tempo);


    QSlider *positionSlider;
    QLabel *positionLabel;

    QLabel *pitchLabel;
    QLabel *tempoLabel;

    QString baseDir;
    QToolButton *openButton;
    QToolButton *playButton;
    QToolButton *pauseButton;
    QToolButton *stopButton;
    QToolButton *pitchUpButton;
    QToolButton *pitchDownButton;
    QToolButton *slowDownButton;
    QToolButton *speedUpButton;

    QToolButton *start_backicon;
    QToolButton *start_forwardicon;
    QToolButton *start_gettime;

    QToolButton *stop_backicon;
    QToolButton *stop_forwardicon;
    QToolButton *stop_gettime;

    QLabel *loopStart;
    QLabel *loopStop;

    QTime startTime;
    QTime stopTime;

};

#endif // MAINWINDOW_H
