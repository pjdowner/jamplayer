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

#include "player.h"
#include <QGlib/Connect>
#include <QGlib/Error>
#include <QGst/Pipeline>
#include <QGst/Parse>
#include <QGst/Bus>
#include <QGst/Message>
#include <QGst/ClockTime>
#include <QGst/Query>
#include <QGst/Event>
/*
#include "player.h"
#include <QDir>
#include <QUrl>
#include <QGlib/Connect>
#include <QGlib/Error>
#include <QGst/Pipeline>
#include <QGst/ElementFactory>
#include <QGst/Bus>
#include <QGst/Message>
#include <QGst/Parse>
#include <QGst/Query>
#include <QGst/ClockTime>
#include <QGst/Event>
#include <QGst/StreamVolume>
*/

Player::Player (QObject *parent) :
        QObject(parent)
{
    connect(&jam_positionTimer, SIGNAL(timeout()), this, SIGNAL(positionChanged()));
}


Player::~Player()
{
    if (jam_pipeline) {
        jam_pipeline->setState(QGst::StateNull);
    }
}


QGst::State Player::state() const
{
    return jam_pipeline ? jam_pipeline->currentState() : QGst::StateNull;
}

float Player::getPitch() const
{
    QGst::ElementPtr t = jam_pipeline->getElementByName("t");

    qDebug() << t->property("pitch");

    QGlib::Value p = t->property("pitch");
    float f;

    bool ok;
    f = p.get<float>(&ok);

    return f;
}

void Player::setPitch(float p) const
{

    QGst::ElementPtr t = jam_pipeline->getElementByName("t");
    t->setProperty("pitch", p);

}


QTime Player::length() const
{
    if (jam_pipeline) {
        //here we query the pipeline about the content's duration
        //and we request that the result is returned in time format
        QGst::DurationQueryPtr query = QGst::DurationQuery::create(QGst::FormatTime);
        jam_pipeline->query(query);
        return QGst::ClockTime(query->duration()).toTime();
    } else {
        return QTime(0,0);
    }
}

QTime Player::position() const
{
    if (jam_pipeline) {
        //here we query the pipeline about its position
        //and we request that the result is returned in time format
        QGst::PositionQueryPtr query = QGst::PositionQuery::create(QGst::FormatTime);
        jam_pipeline->query(query);
        return QGst::ClockTime(query->position()).toTime();
    } else {
        return QTime(0,0);
    }
}

void Player::setPosition(const QTime & pos)
{
    QGst::SeekEventPtr evt = QGst::SeekEvent::create(
        1.0, QGst::FormatTime, QGst::SeekFlagFlush,
        QGst::SeekTypeSet, QGst::ClockTime::fromTime(pos),
        QGst::SeekTypeNone, QGst::ClockTime::None
    );

    jam_pipeline->sendEvent(evt);
}

void Player::setLocation(const QString & location)
{

    if (!jam_pipeline) {

        QString pipe1Descr = QString(   "filesrc location=\"%1\" ! "
                                        "decodebin ! "
                                        "audioconvert ! "
                                        "pitch name=t tempo=1.0 ! "
                                        "audiopanorama panorama=-1.00 ! "
                                        "autoaudiosink "
                                        ).arg(location);

        jam_pipeline = QGst::Parse::launch(pipe1Descr).dynamicCast<QGst::Pipeline>();

        if (jam_pipeline) {

            QGst::BusPtr bus = jam_pipeline->bus();
            bus->addSignalWatch();
            QGlib::connect(bus, "message", this, &Player::busMessageRecv);
            currentFile = location;

        } else {
            qCritical() << "Failed to create the pipeline";
        }

    }

    if (jam_pipeline) {
        currentFile = location;
        jam_pipeline->setProperty("location", location);
    }
}

void Player::busMessageRecv(const QGst::MessagePtr & message)
{
    switch (message->type()) {
    case QGst::MessageDurationChanged:
        Q_EMIT durationChanged();
        break;
    case QGst::MessageEos:
        stop();
        break;
    case QGst::MessageError:
        qCritical() << message.staticCast<QGst::ErrorMessage>()->error();
        stop();
        break;
    case QGst::MessageStateChanged:
        if (message->source() == jam_pipeline) {
            stateChange(message.staticCast<QGst::StateChangedMessage>());
        }
        break;
    default:
        break;
    }
}
void Player::stop()
{
    if (jam_pipeline) {
        jam_pipeline->setState(QGst::StateNull);
        jam_pipeline.clear();

        Q_EMIT stateChanged();
    }
}

void Player::play()
{
    if (!jam_pipeline) {
        qDebug() << "no pipeline";
        setLocation(currentFile);
    }

    if (jam_pipeline) {
        qDebug() << "setState playing";
        jam_pipeline->setState(QGst::StatePlaying);
    }
}

void Player::pause()
{
    if (jam_pipeline) {
        if (state() == QGst::StatePaused) {
                jam_pipeline->setState(QGst::StatePlaying);
        } else {
                jam_pipeline->setState(QGst::StatePaused);
        }
    }
}

void Player::pitchDown()
{

    QGst::ElementPtr t = jam_pipeline->getElementByName("t");

    qDebug() << t->property("pitch");

    QGlib::Value p = t->property("pitch");
    float f;

    bool ok;
    f = p.get<float>(&ok);


    f = f - 0.05;



    t->setProperty("pitch", f);
}

void Player::pitchUp()
{

    QGst::ElementPtr t = jam_pipeline->getElementByName("t");

    qDebug() << t->property("pitch");

    QGlib::Value p = t->property("pitch");
    float f;

    bool ok;
    f = p.get<float>(&ok);

    f = f + 0.05;
    t->setProperty("pitch", f);
}

void Player::stateChange(const QGst::StateChangedMessagePtr & playerState)
{
    switch (playerState->newState()) {
    case QGst::StatePlaying:
        jam_positionTimer.start(100);
        break;
    case QGst::StatePaused:
        if(playerState->oldState() == QGst::StatePlaying) {
            jam_positionTimer.stop();
        }
        break;
    default:
        break;
    }

    Q_EMIT stateChanged();
}


void Player::setTempo(float tempo)
{
    if (jam_pipeline) {

        qDebug() << jam_pipeline->listProperties();
        //m_pipeline->setProperty("tempo", tempo);
        QGst::ElementPtr t = jam_pipeline->getElementByName("t");

        QList<QGlib::ParamSpecPtr> ptr;

        ptr = t->listProperties();

        for (int i = 0; i < ptr.size(); ++i) {
            qDebug()<<ptr.at(i)->name();

        }

        QGlib::ParamSpecPtr p = t->findProperty("tempo");

        qDebug() << t->property("tempo");

        //ptr t->listProperties();
        t->setProperty("tempo", tempo);

    }
}

float Player::getTempo(void)
{
    if (jam_pipeline) {

        qDebug() <<jam_pipeline->listProperties();

        QGst::ElementPtr t = jam_pipeline->getElementByName("t");

        QGlib::Value tempo = t->property("tempo");

        qDebug() << tempo;

        float f;
        bool ok;
        f = tempo.get<float>(&ok);

        return f;

    }
    return 0;
}



