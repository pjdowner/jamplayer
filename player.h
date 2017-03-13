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

private:

    QGst::PipelinePtr jam_pipeline;
    void busMessageRecv(const QGst::MessagePtr & message);
    void stateChange(const QGst::StateChangedMessagePtr & playerState);
    QTimer jam_positionTimer;
};

#endif
