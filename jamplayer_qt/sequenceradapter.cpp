
#include "sequenceradapter.h"
#include "defs.h"


#include <QStringList>


SequencerAdapter::SequencerAdapter(QObject *parent) :
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



SequencerAdapter::~SequencerAdapter() 
{
    m_Client->stopSequencerInput();    
    m_Port->detach();
    m_Client->close();
}


/********* pjd ********/


void SequencerAdapter::subscription(MidiPort* port, Subscription* subs){
    qDebug() << "Subscription made from"
             << subs->getSender()->client << ":"
             << subs->getSender()->port;

}


bool SequencerAdapter::stopped()
{
    QReadLocker locker(&m_mutex);
    return m_Stopped;
}

void SequencerAdapter::stop()
{
    QWriteLocker locker(&m_mutex);
    m_Stopped = true;
}

#ifdef USE_QEVENTS
void SequencerAdapter::customEvent(QEvent *ev)
{
    if (ev->type() == SequencerEventType) {
        SequencerEvent* sev = static_cast<SequencerEvent*>(ev);
        if (sev != NULL) {
            dumpEvent(sev);
        }
    }
}
#else
void SequencerAdapter::sequencerEvent(SequencerEvent *ev)
{
    dumpEvent(ev);
    delete ev;
}
#endif


void SequencerAdapter::run()
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
/*
        m_Stopped = false;
        while (!stopped()) {
#ifdef USE_QEVENTS
            QApplication::sendPostedEvents();
#endif
            sleep(1);
        }
#ifdef WANT_TIMESTAMPS
        m_Queue->stop();
#endif
        m_Client->stopSequencerInput();
    } catch (const SequencerError& err) {
        qDebug() << "SequencerError exception. Error code: " << err.code()
             << " (" << err.qstrError() << ")" << endl;
        qDebug() << "Location: " << err.location() << endl;
        throw err;
    }
*/
}


void SequencerAdapter::dumpEvent(SequencerEvent* sev)
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
    case SND_SEQ_EVENT_NOTEON: {
        NoteOnEvent* e = static_cast<NoteOnEvent*>(sev);
        if (e != NULL) {
            qDebug() << qSetFieldWidth(23) << left << "Note on";
            qDebug() << qSetFieldWidth(2) << right << e->getChannel() << " ";
            qDebug() << qSetFieldWidth(3) << e->getKey() << " ";
            qDebug() << qSetFieldWidth(3) << e->getVelocity();
        }
        break;
    }
    case SND_SEQ_EVENT_NOTEOFF: {
        NoteOffEvent* e = static_cast<NoteOffEvent*>(sev);
        if (e != NULL) {
            qDebug() << qSetFieldWidth(23) << left << "Note off";
            qDebug() << qSetFieldWidth(2) << right << e->getChannel() << " ";
            qDebug() << qSetFieldWidth(3) << e->getKey() << " ";
            qDebug() << qSetFieldWidth(3) << e->getVelocity();
        }
        break;
    }
    case SND_SEQ_EVENT_KEYPRESS: {
        KeyPressEvent* e = static_cast<KeyPressEvent*>(sev);
        if (e != NULL) {
            qDebug() << qSetFieldWidth(23) << left << "Polyphonic aftertouch";
            qDebug() << qSetFieldWidth(2) << right << e->getChannel() << " ";
            qDebug() << qSetFieldWidth(3) << e->getKey() << " ";
            qDebug() << qSetFieldWidth(3) << e->getVelocity();
        }
        break;
    }
    case SND_SEQ_EVENT_CONTROL14:
    case SND_SEQ_EVENT_NONREGPARAM:
    case SND_SEQ_EVENT_REGPARAM:
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
        }
        break;
    }
    case SND_SEQ_EVENT_PGMCHANGE: {
        ProgramChangeEvent* e = static_cast<ProgramChangeEvent*>(sev);
        if (e != NULL) {
            qDebug() << qSetFieldWidth(23) << left << "Program change";
            qDebug() << qSetFieldWidth(2) << right << e->getChannel() << " ";
            qDebug() << qSetFieldWidth(3) << e->getValue();
        }
        break;
    }
    case SND_SEQ_EVENT_CHANPRESS: {
        ChanPressEvent* e = static_cast<ChanPressEvent*>(sev);
        if (e != NULL) {
            qDebug() << qSetFieldWidth(23) << left << "Channel aftertouch";
            qDebug() << qSetFieldWidth(2) << right << e->getChannel() << " ";
            qDebug() << qSetFieldWidth(3) << e->getValue();
        }
        break;
    }
    case SND_SEQ_EVENT_PITCHBEND: {
        PitchBendEvent* e = static_cast<PitchBendEvent*>(sev);
        if (e != NULL) {
            qDebug() << qSetFieldWidth(23) << left << "Pitch bend";
            qDebug() << qSetFieldWidth(2) << right << e->getChannel() << " ";
            qDebug() << qSetFieldWidth(5) << e->getValue();
        }
        break;
    }
    case SND_SEQ_EVENT_SONGPOS: {
        ValueEvent* e = static_cast<ValueEvent*>(sev);
        if (e != NULL) {
            qDebug() << qSetFieldWidth(26) << left << "Song position pointer" << qSetFieldWidth(0);
            qDebug() << e->getValue();
        }
        break;
    }
    case SND_SEQ_EVENT_SONGSEL: {
        ValueEvent* e = static_cast<ValueEvent*>(sev);
        if (e != NULL) {
            qDebug() << qSetFieldWidth(26) << left << "Song select" << qSetFieldWidth(0);
            qDebug() << e->getValue();
        }
        break;
    }
    case SND_SEQ_EVENT_QFRAME: {
        ValueEvent* e = static_cast<ValueEvent*>(sev);
        if (e != NULL) {
            qDebug() << qSetFieldWidth(26) << left << "MTC quarter frame" << qSetFieldWidth(0);
            qDebug() << e->getValue();
        }
        break;
    }
    case SND_SEQ_EVENT_TIMESIGN: {
        ValueEvent* e = static_cast<ValueEvent*>(sev);
        if (e != NULL) {
            qDebug() << qSetFieldWidth(26) << left << "SMF time signature" << qSetFieldWidth(0);
            qDebug() << hex << e->getValue();
            qDebug() << dec;
        }
        break;
    }
    case SND_SEQ_EVENT_KEYSIGN: {
        ValueEvent* e = static_cast<ValueEvent*>(sev);
        if (e != NULL) {
            qDebug() << qSetFieldWidth(26) << left << "SMF key signature" << qSetFieldWidth(0);
            qDebug() << hex << e->getValue();
            qDebug() << dec;
        }
        break;
    }
    case SND_SEQ_EVENT_SETPOS_TICK: {
        QueueControlEvent* e = static_cast<QueueControlEvent*>(sev);
        if (e != NULL) {
            qDebug() << qSetFieldWidth(26) << left << "Set tick queue pos." << qSetFieldWidth(0);
            qDebug() << e->getQueue();
        }
        break;
    }
    case SND_SEQ_EVENT_SETPOS_TIME: {
        QueueControlEvent* e = static_cast<QueueControlEvent*>(sev);
        if (e != NULL) {
            qDebug() << qSetFieldWidth(26) << left << "Set rt queue pos." << qSetFieldWidth(0);
            qDebug() << e->getQueue();
        }
        break;
    }
    case SND_SEQ_EVENT_TEMPO: {
        TempoEvent* e = static_cast<TempoEvent*>(sev);
        if (e != NULL) {
            qDebug() << qSetFieldWidth(26) << left << "Set queue tempo";
            qDebug() << qSetFieldWidth(3) << right << e->getQueue() << qSetFieldWidth(0) << " ";
            qDebug() << e->getValue();
        }
        break;
    }
    case SND_SEQ_EVENT_QUEUE_SKEW: {
        QueueControlEvent* e = static_cast<QueueControlEvent*>(sev);
        if (e != NULL) {
            qDebug() << qSetFieldWidth(26) << left << "Queue timer skew" << qSetFieldWidth(0);
            qDebug() << e->getQueue();
        }
        break;
    }
    case SND_SEQ_EVENT_START:
        qDebug() << left << "Start";
        break;
    case SND_SEQ_EVENT_STOP:
        qDebug() << left << "Stop";
        break;
    case SND_SEQ_EVENT_CONTINUE:
        qDebug() << left << "Continue";
        break;
    case SND_SEQ_EVENT_CLOCK:
        qDebug() << left << "Clock";
        break;
    case SND_SEQ_EVENT_TICK:
        qDebug() << left << "Tick";
        break;
    case SND_SEQ_EVENT_TUNE_REQUEST:
        qDebug() << left << "Tune request";
        break;
    case SND_SEQ_EVENT_RESET:
        qDebug() << left << "Reset";
        break;
    case SND_SEQ_EVENT_SENSING:
        qDebug() << left << "Active Sensing";
        break;
    case SND_SEQ_EVENT_CLIENT_START: {
        ClientEvent* e = static_cast<ClientEvent*>(sev);
        if (e != NULL) {
            qDebug() << qSetFieldWidth(26) << left << "Client start"
                 << qSetFieldWidth(0) << e->getClient();
        }
        break;
    }
    case SND_SEQ_EVENT_CLIENT_EXIT: {
        ClientEvent* e = static_cast<ClientEvent*>(sev);
        if (e != NULL) {
            qDebug() << qSetFieldWidth(26) << left << "Client exit"
                 << qSetFieldWidth(0) << e->getClient();
        }
        break;
    }
    case SND_SEQ_EVENT_CLIENT_CHANGE: {
        ClientEvent* e = static_cast<ClientEvent*>(sev);
        if (e != NULL) {
            qDebug() << qSetFieldWidth(26) << left << "Client changed"
                 << qSetFieldWidth(0) << e->getClient();
        }
        break;
    }
    case SND_SEQ_EVENT_PORT_START: {
        PortEvent* e = static_cast<PortEvent*>(sev);
        if (e != NULL) {
            qDebug() << qSetFieldWidth(26) << left << "Port start" << qSetFieldWidth(0);
            qDebug() << e->getClient() << ":" << e->getPort();
        }
        break;
    }
    case SND_SEQ_EVENT_PORT_EXIT: {
        PortEvent* e = static_cast<PortEvent*>(sev);
        if (e != NULL) {
            qDebug() << qSetFieldWidth(26) << left << "Port exit" << qSetFieldWidth(0);
            qDebug() << e->getClient() << ":" << e->getPort();
        }
        break;
    }
    case SND_SEQ_EVENT_PORT_CHANGE: {
        PortEvent* e = static_cast<PortEvent*>(sev);
        if (e != NULL) {
            qDebug() << qSetFieldWidth(26) << left << "Port changed" << qSetFieldWidth(0);
            qDebug() << e->getClient() << ":" << e->getPort();
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
    default:
        qDebug() << qSetFieldWidth(26) << "Unknown event type" << qSetFieldWidth(0);
        qDebug() << sev->getSequencerType();
    };
    qDebug() << qSetFieldWidth(0) << endl;
}

void SequencerAdapter::subscribe(const QString& portName)
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

void SequencerAdapter::handleSequencerEvent(SequencerEvent *ev)
{
    int when = 0;
    switch (ev->getSequencerType()) {
    case SND_SEQ_EVENT_USR0:
        when = ev->getTick() + m_patternDuration;
        //if (m_patternMode)
            //metronome_grid_pattern(when);
        //else
            //metronome_simple_pattern(when);
        m_bar++;
        m_beat = 0;
        break;
    case SND_SEQ_EVENT_USR1:
        m_beat++;
        emit signalUpdate(m_bar, m_beat);
        break;
    case SND_SEQ_EVENT_START:
        emit signalPlay();
        break;
    case SND_SEQ_EVENT_CONTINUE:
        emit signalCont();
        break;
    case SND_SEQ_EVENT_STOP:
        emit signalStop();
        break;
    case SND_SEQ_EVENT_SYSEX:
        //parse_sysex(ev);
        break;
    case SND_SEQ_EVENT_NOTEON:
        //metronome_note_output(ev);
        break;
    case SND_SEQ_EVENT_NOTEOFF:
        //metronome_event_output(ev);
        break;
    }
    delete ev;
}


/*********** eopjd *********/

