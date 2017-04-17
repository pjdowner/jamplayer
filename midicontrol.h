#ifndef MIDICONTROL_H
#define MIDICONTROL_H

/*
    Copyright (C) 2017, Philip Downer <phil@pjd.me.uk>

    Originally based on KMetronome - ALSA Sequencer based MIDI metronome
    Copyright (C) 2006-2016, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <drumstick/alsaqueue.h>
#include <drumstick/alsaevent.h>
#include <drumstick/alsaport.h>
#include <drumstick/alsaqueue.h>
#include <drumstick/alsaclient.h>
#include <drumstick/subscription.h>
#include "player.h"

namespace drumstick {
    class MidiPort;
    class MidiQueue;
    class SequencerEvent;
};

#include <drumstick/alsaclient.h>

using namespace drumstick;


class midicontrol : public QObject
{

    Q_OBJECT

public:

    void dumpEvent(SequencerEvent* ev);
    void subscribe(const QString& portName);
    void run();

    midicontrol(QObject *parent);
    virtual ~midicontrol();


public slots:
    void subscription(MidiPort* port, Subscription* subs);

#ifdef USE_QEVENTS
protected:
    virtual void customEvent( QEvent *ev );
#else
    void sequencerEvent( SequencerEvent* ev );
#endif

signals:
    void keyPressed(int);

private:
    bool m_Stopped;
    QReadWriteLock m_mutex;

    MidiClient* m_Client;
    MidiPort* m_Port;
    MidiQueue* m_Queue;


};

#endif // MIDICONTROL_H
