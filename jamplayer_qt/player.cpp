/*
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

Player::Player(QWidget *parent)
    : QGst::Ui::VideoWidget(parent)
{
    //this timer is used to tell the ui to change its position slider & label
    //every 100 ms, but only when the pipeline is playing
    connect(&m_positionTimer, SIGNAL(timeout()), this, SIGNAL(positionChanged()));
}

Player::~Player()
{
    if (m_pipeline) {
        m_pipeline->setState(QGst::StateNull);
        stopPipelineWatch();
    }
}

void Player::setUri(const QString & uri)
{
    QString realUri = uri;

    //if uri is not a real uri, assume it is a file path
    if (realUri.indexOf("://") < 0) {
        realUri = QUrl::fromLocalFile(realUri).toEncoded();
    }

    if (!m_pipeline) {

        /*
        m_pipeline = QGst::Pipeline::create("audio");
        source = QGst::ElementFactory::make("filesrc", "filesource");
        decoder = QGst::ElementFactory::make("decodebin", "decoder");
        convert = QGst::ElementFactory::make("audioconvert", "convert");
        pitch = QGst::ElementFactory::make("pitch", "pitch");
        sink = QGst::ElementFactory::make("autoaudiosink", "sink");

        source->setProperty("location", uri);
        m_pipeline->add(source);
        m_pipeline->add(decoder);
        m_pipeline->add(convert);
        m_pipeline->add(pitch);
        m_pipeline->add(sink);

        m_pipeline->link("filesource", decoder);
        //m_pipeline->link("convert", pitch);
       // m_pipeline->link("pitch", sink);
        m_pipeline->link("decoder", sink);
*/

        currentFile = uri;

        QString pipe1Descr = QString(   "filesrc location=\"%1\" ! "
                                        "decodebin ! "
                                        "audioconvert ! "
                                        "pitch name=t tempo=1.0 ! "
                                        "autoaudiosink "
                                        ).arg(uri);

        m_pipeline = QGst::Parse::launch(pipe1Descr).dynamicCast<QGst::Pipeline>();



        if (m_pipeline) {
            watchPipeline(m_pipeline);

            QGst::BusPtr bus = m_pipeline->bus();
            bus->addSignalWatch();
            QGlib::connect(bus, "message", this, &Player::onBusMessage);

           // QGlib::connect(decoder, "pad-added", this, &Player::on_pad_added);
        } else {
            qCritical() << "Failed to create the pipeline";
        }

    }

    if (m_pipeline) {
        m_pipeline->setProperty("location", uri);
    }
}

QTime Player::position() const
{
    if (m_pipeline) {
        //here we query the pipeline about its position
        //and we request that the result is returned in time format
        QGst::PositionQueryPtr query = QGst::PositionQuery::create(QGst::FormatTime);
        m_pipeline->query(query);
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

    m_pipeline->sendEvent(evt);
}

int Player::volume() const
{
    if (m_pipeline) {
        QGst::StreamVolumePtr svp =
            m_pipeline.dynamicCast<QGst::StreamVolume>();

        if (svp) {
            return svp->volume(QGst::StreamVolumeFormatCubic) * 10;
        }
    }

    return 0;
}

void Player::setTempo(float tempo)
{
    if (m_pipeline) {

        qDebug() <<m_pipeline->listProperties();
        //m_pipeline->setProperty("tempo", tempo);
        QGst::ElementPtr t = m_pipeline->getElementByName("t");

        QList<QGlib::ParamSpecPtr> ptr;

        ptr = t->listProperties();


        for (int i = 0; i < ptr.size(); ++i) {
            qDebug()<<ptr.at(i)->name();
        }

        //ptr t->listProperties();
        t->setProperty("tempo", tempo);

    }
}

void Player::setVolume(int volume)
{



    if (m_pipeline) {
        QGst::StreamVolumePtr svp =
            m_pipeline.dynamicCast<QGst::StreamVolume>();

        if(svp) {
            svp->setVolume((double)volume / 10, QGst::StreamVolumeFormatCubic);
        }
    }
}

QTime Player::length() const
{
    if (m_pipeline) {
        //here we query the pipeline about the content's duration
        //and we request that the result is returned in time format
        QGst::DurationQueryPtr query = QGst::DurationQuery::create(QGst::FormatTime);
        m_pipeline->query(query);
        return QGst::ClockTime(query->duration()).toTime();
    } else {
        return QTime(0,0);
    }
}

QGst::State Player::state() const
{
    return m_pipeline ? m_pipeline->currentState() : QGst::StateNull;
}

void Player::play()
{
    if (!m_pipeline) {
        setUri(currentFile);
    }

    if (m_pipeline) {
        m_pipeline->setState(QGst::StatePlaying);
    }
}

void Player::pause()
{
    if (m_pipeline) {
        m_pipeline->setState(QGst::StatePaused);
    }
}

void Player::stop()
{
    if (m_pipeline) {
        m_pipeline->setState(QGst::StateNull);
        m_pipeline.clear();
        //once the pipeline stops, the bus is flushed so we will
        //not receive any StateChangedMessage about this.
        //so, to inform the ui, we have to emit this signal manually.
        Q_EMIT stateChanged();
    }
}

void Player::on_pad_added(QGst::ElementPtr elem) {

    qDebug() << elem->name();

}

void Player::onBusMessage(const QGst::MessagePtr & message)
{
    switch (message->type()) {
    case QGst::MessageEos: //End of stream. We reached the end of the file.
        stop();
        break;
    case QGst::MessageError: //Some error occurred.
        qCritical() << message.staticCast<QGst::ErrorMessage>()->error();
        stop();
        break;
    case QGst::MessageStateChanged: //The element in message->source() has changed state
        if (message->source() == m_pipeline) {
            handlePipelineStateChange(message.staticCast<QGst::StateChangedMessage>());
        }
        break;
    default:
        break;
    }
}

void Player::handlePipelineStateChange(const QGst::StateChangedMessagePtr & scm)
{
    switch (scm->newState()) {
    case QGst::StatePlaying:
        //start the timer when the pipeline starts playing
        m_positionTimer.start(100);
        break;
    case QGst::StatePaused:
        //stop the timer when the pipeline pauses
        if(scm->oldState() == QGst::StatePlaying) {
            m_positionTimer.stop();
        }
        break;
    default:
        break;
    }

    Q_EMIT stateChanged();
}

#include "moc_player.cpp"
