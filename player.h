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

#ifndef PLAYER_H
#define PLAYER_H

#include <QTimer>
#include <QTime>
#include <QGst/Pipeline>
#include <QGst/Ui/VideoWidget>
#include "music.h"

class Player : public QObject
{
    Q_OBJECT
public:
    Player(QObject *parent = 0);
    ~Player();

    void setLocation(const QString & location);

    QString currentFile;
    QTime position() const;
    void setPosition(const QTime & pos);
    QTime length() const;
    void setPitch(float) const;
    float getPitch() const;
    QGst::State state() const;
    void setTempo(float tempo);
    float getTempo(void);

public Q_SLOTS:
    void stop();
    void play();
    void pause();
    void pitchDown();
    void pitchUp();

Q_SIGNALS:
    void positionChanged();
    void stateChanged();
    void durationChanged();
    void loopCheck();

private:

    QGst::PipelinePtr jam_pipeline;
    void busMessageRecv(const QGst::MessagePtr & message);
    void stateChange(const QGst::StateChangedMessagePtr & playerState);
    QTimer jam_positionTimer;
};

#endif
