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

#include "midicontrol.h"

midicontrol::midicontrol(QObject *parent) :
    QObject(parent)
{

    m_Client = new MidiClient(this);
    m_Client->open();
    m_Client->setClientName("DumpMIDI");

#ifndef USE_QEVENTS // using signals instead
    connect( m_Client, SIGNAL(eventReceived(SequencerEvent*)),
                       SLOT(sequencerEvent(SequencerEvent*)),
                       Qt::DirectConnection );
#endif
    m_Port = new MidiPort(this);
    m_Port->attach( m_Client );
    m_Port->setPortName("DumpMIDI port");
    m_Port->setCapability( SND_SEQ_PORT_CAP_WRITE |
                           SND_SEQ_PORT_CAP_SUBS_WRITE );
    m_Port->setPortType( SND_SEQ_PORT_TYPE_APPLICATION |
                         SND_SEQ_PORT_TYPE_MIDI_GENERIC );
#ifdef WANT_TIMESTAMPS
    m_Queue = m_Client->createQueue("DumpMIDI");
    m_Port->setTimestamping(true);
    //m_Port->setTimestampReal(true);
    m_Port->setTimestampQueue(m_Queue->getId());
#endif
    connect( m_Port, SIGNAL(subscribed(MidiPort*,Subscription*)),
                     SLOT(subscription(MidiPort*,Subscription*)));
    qDebug() << "Trying to subscribe from Announce";
    m_Port->subscribeFromAnnounce();

}

#ifdef USE_QEVENTS
void midicontrol::customEvent(QEvent *ev)
{
    if (ev->type() == SequencerEventType) {
        SequencerEvent* sev = static_cast<SequencerEvent*>(ev);
        if (sev != NULL) {
            dumpEvent(sev);
        }
    }
}
#else
void midicontrol::sequencerEvent(SequencerEvent *ev)
{
    dumpEvent(ev);
    delete ev;
}
#endif

void midicontrol::subscription(MidiPort* port, Subscription* subs){
    qDebug() << "Subscription made from"
             << subs->getSender()->client << ":"
             << subs->getSender()->port; //<< " midiport = " << port;
}

void midicontrol::dumpEvent(SequencerEvent* sev)
{
#ifdef WANT_TIMESTAMPS
    qDebug() << qSetFieldWidth(8) << right << sev->getTick();
    /* More timestamp options:
    qDebug() << sev->getRealTimeSecs();
    qDebug() << sev->getRealTimeNanos(); */
    /* Getting the time from the queue status object;
    QueueStatus sts = m_Queue->getStatus();
    qDebug() << qSetFieldWidth(8) << right << sts.getClockTime();
    qDebug() << sts.getTickTime(); */
    qDebug() << qSetFieldWidth(0) << " ";
#endif

    qDebug() << qSetFieldWidth(3) << right << sev->getSourceClient() << qSetFieldWidth(0) << ":";
    qDebug() << qSetFieldWidth(3) << left << sev->getSourcePort() << qSetFieldWidth(0) << " ";

    switch (sev->getSequencerType()) {
        case SND_SEQ_EVENT_CONTROLLER: {
            ControllerEvent* e = static_cast<ControllerEvent*>(sev);
            if (e != NULL) {
                qDebug() << qSetFieldWidth(23) << left << "Control change";
                qDebug() << qSetFieldWidth(2) << right << e->getChannel() << " ";
                qDebug() << qSetFieldWidth(3) << e->getParam() << " ";
                qDebug() << qSetFieldWidth(3) << e->getValue();
            }

            unsigned int k = e->getParam();


            qDebug() << "k = " << k;
            if (k == 20) {
                qDebug() << "PJD rocks!";
                emit keyPressed(1);

            } else if (k == 22) {
                qDebug() << "PJD rocks! 2";
                emit keyPressed(2);
            } else if (k == 21) {
                qDebug() << "PJD time travel!";
                emit keyPressed(3);
            }
            break;
        }
        case SND_SEQ_EVENT_PORT_SUBSCRIBED: {
            SubscriptionEvent* e = static_cast<SubscriptionEvent*>(sev);
            if (e != NULL) {
                qDebug() << qSetFieldWidth(26) << left << "Port subscribed" << qSetFieldWidth(0);
                qDebug() << e->getSenderClient() << ":" << e->getSenderPort() << " -> ";
                qDebug() << e->getDestClient() << ":" << e->getDestPort();
            }
            break;
        }
        case SND_SEQ_EVENT_PORT_UNSUBSCRIBED: {
            SubscriptionEvent* e = static_cast<SubscriptionEvent*>(sev);
            if (e != NULL) {
                qDebug() << qSetFieldWidth(26) << left << "Port unsubscribed" << qSetFieldWidth(0);
                qDebug() << e->getSenderClient() << ":" << e->getSenderPort() << " -> ";
                qDebug() << e->getDestClient() << ":" << e->getDestPort();
            }
            break;
        }
        case SND_SEQ_EVENT_SYSEX: {
            SysExEvent* e = static_cast<SysExEvent*>(sev);
            if (e != NULL) {
                qDebug() << qSetFieldWidth(26) << left << "System exclusive" << qSetFieldWidth(0);
                unsigned int i;

                for (i = 0; i < e->getLength(); ++i) {

                    qDebug() << hex << (unsigned char) e->getData()[i] << " ";
                }
                qDebug() << dec;



            }
            break;
        }
        default: {
            qDebug() << qSetFieldWidth(26) << "Unknown event type" << qSetFieldWidth(0);
            qDebug() << sev->getSequencerType();
        }
    };
    qDebug() << qSetFieldWidth(0) << endl;
}


void midicontrol::run()
{
    //qDebug() << "Press Ctrl+C to exit" << endl;
#ifdef WANT_TIMESTAMPS
    qDebug() << "___Ticks ";
#endif
    qDebug() << "Source_ Event_________________ Ch _Data__" << endl;
//    try {
#ifdef USE_QEVENTS
        m_Client->addListener(this);
        m_Client->setEventsEnabled(true);
#endif
        m_Client->setRealTimeInput(false);
        m_Client->startSequencerInput();
#ifdef WANT_TIMESTAMPS
        m_Queue->start();
#endif

}

QStringList midicontrol::inputConnections()
{
    PortInfoList inputs(m_Client->getAvailableInputs());
    return list_ports(inputs);
}

QStringList midicontrol::outputConnections()
{
    PortInfoList outputs(m_Client->getAvailableOutputs());
    return list_ports(outputs);
}

QStringList midicontrol::list_ports(PortInfoList& refs)
{
    QStringList lst;
    foreach(PortInfo p, refs) {
        lst += QString("%1:%2").arg(p.getClientName()).arg(p.getPort());
        qDebug() << QString("%1:%2").arg(p.getClientName()).arg(p.getPort());
    }
    return lst;
}


void midicontrol::subscribe(const QString& portName)
{
    try {
        qDebug() << "Trying to subscribe" << portName.toLocal8Bit().data();
        m_Port->subscribeFrom(portName);
    } catch (const SequencerError& err) {
        qDebug() << "SequencerError exception. Error code: " << err.code()
             << " (" << err.qstrError() << ")" << endl;
        qDebug() << "Location: " << err.location() << endl;
        throw err;
    }
}

midicontrol::~midicontrol()
{

}
