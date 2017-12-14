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

#include "midicontrol.h"
#include "mainwindow.h"
#include <QBoxLayout>
#include <QToolButton>
#include <QFileDialog>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QInputDialog>
#include <QFile>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    jam_player = new Player(this);
    connect(jam_player, SIGNAL(stateChanged()), this, SLOT(onStateChanged()));

    MD = new Music(this);



    QVBoxLayout *appLayout = new QVBoxLayout;
    appLayout->setContentsMargins(0, 0, 0, 0);
    createUI(appLayout);


    QWidget *window = new QWidget();
    window->setLayout(appLayout);

    // Set QWidget as the central layout of the main window
    setCentralWidget(window);

}



void MainWindow::readJson(QJsonObject &json)
{
    QJsonArray songs = json["songs"].toArray();

}

QToolButton *MainWindow::initButton(QStyle::StandardPixmap icon, const QString & tip,
                                  QObject *dstobj, const char *slot_method, QLayout *layout)
{
    QToolButton *button = new QToolButton;
    button->setIcon(style()->standardIcon(icon));
    button->setIconSize(QSize(36, 36));
    button->setToolTip(tip);
    connect(button, SIGNAL(clicked()), dstobj, slot_method);
    layout->addWidget(button);

    return button;
}

void MainWindow::onStateChanged()
{

}

void MainWindow::updateLoopList(QJsonArray loops)
{
    loopList->clear();

    QListWidgetItem *all = new QListWidgetItem("All", loopList);
    loopList->setCurrentItem(all);

    for (int i=0; i < loops.size(); i++) {
        if (loops[i].isObject()) {
            QJsonObject section = loops[i].toObject();
            qDebug() << section.value("name").toString();
            new QListWidgetItem(section.value("name").toString(), loopList);
        }
    }
}

void MainWindow::updateList(QJsonArray ja)
{
    songList->clear();
    for (int i =0; i < ja.size(); i++) {
        if (ja[i].isObject()) {
            QJsonObject jo = ja[i].toObject();
            qDebug() << jo.value("name").toString();
            new QListWidgetItem(jo.value("name").toString(), songList );
            if (jo.value("loops").isArray()){
                updateLoopList(jo.value("loops").toArray());
            }
        }
    }

}

void MainWindow::createUI(QBoxLayout *appLayout)
{
    const QString errorstr = "Fatal error from the ALSA sequencer. "
            "This usually happens when the kernel doesn't have ALSA support, "
            "or the device node (/dev/snd/seq) doesn't exists, "
            "or the kernel module (snd_seq) is not loaded. "
            "Please check your ALSA/MIDI configuration.";
    try {
        mControl = new midicontrol(this);

        QString portName = "FBV Express Mk II:0";
        mControl->subscribe(portName);
        QStringList devices = mControl->inputConnections();
        mControl->run();
    }
    catch (const SequencerError& ex) {
        qDebug() << errorstr + " Returned error was: " + ex.qstrError() << endl;
    } catch (...) {
        qDebug() << errorstr << endl;
    }


    baseDir = QLatin1String(".");
    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->addStretch();

    positionLabel = new QLabel();
    QFont f( "Arial", 20);
    positionLabel->setFont(f);
    positionLabel->setText("hh:mm:ss.zzz");

    positionSlider = new QSlider(Qt::Horizontal);
    positionSlider->setTickPosition(QSlider::TicksBelow);
    positionSlider->setTickInterval(10);
    positionSlider->setMaximum(1000);

    pitchLabel = new QLabel();
    pitchLabel->setFont(f);
    pitchLabel->setText("Pitch:");

    tempoLabel = new QLabel();
    tempoLabel->setFont(f);
    tempoLabel->setText("Tempo:");

    connect(jam_player, SIGNAL(positionChanged()), this, SLOT(onPositionChanged()));
    connect(positionSlider, SIGNAL(sliderMoved(int)), this, SLOT(setPosition(int)));
    connect(mControl, SIGNAL(keyPressed(int)), this, SLOT(keyed(int)));

    songList = new QListWidget(this);
    loopList = new QListWidget(this);

    QJsonArray ja = MD->loadData();
    if ( ja.size() < 1) {
        qDebug() <<"fail";
    }
    updateList(ja);





    QGridLayout *posLayout = new QGridLayout;
    posLayout->setContentsMargins(50,50,50,50);
    posLayout->addWidget(songList, 1,1,1,2, Qt::AlignLeft);
    posLayout->addWidget(loopList,1,1,1,2, Qt::AlignRight);

    posLayout->addWidget(positionSlider, 2, 1, 1, 2, Qt::AlignVCenter);
    posLayout->addWidget(positionLabel, 3, 1, 1, 2, Qt::AlignHCenter|Qt::AlignTop);
    posLayout->addWidget(tempoLabel, 4,1,1,1, Qt::AlignCenter);
    posLayout->addWidget(pitchLabel, 4,2,1,1, Qt::AlignCenter);
    appLayout->addLayout(posLayout);

    openButton = initButton(QStyle::SP_DialogOpenButton, tr("Open File"),
                              this, SLOT(open()), btnLayout);

    playButton = initButton(QStyle::SP_MediaPlay, tr("Play"),
                              this, SLOT(loadFile()), btnLayout);

    pauseButton = initButton(QStyle::SP_MediaPause, tr("Pause"),
                              jam_player, SLOT(pause()), btnLayout);

    stopButton = initButton(QStyle::SP_MediaStop, tr("Stop"),
                              jam_player, SLOT(stop()), btnLayout);

    pitchDownButton = initButton(QStyle::SP_ArrowDown, tr("Pitch Down"),
                              this, SLOT(pitchDown()), btnLayout);

    pitchUpButton = initButton(QStyle::SP_ArrowUp, tr("Pitch Up"),
                              this, SLOT(pitchUp()), btnLayout);

    slowDownButton = initButton(QStyle::SP_ArrowDown, tr("Slow Down"),
                                this, SLOT(slowDown()), btnLayout);

    speedUpButton = initButton(QStyle::SP_ArrowUp, tr("Speed Up"),
                               this, SLOT(speedUp()), btnLayout);

    btnLayout->addStretch();

    appLayout->addLayout(btnLayout);


}

void MainWindow::keyed(int k)
{
    qDebug() << "do something!";
    if (k == 1) {
        slowDown();

    } else if (k == 2) {
        speedUp();

    } else if (k == 3) {
        QTime t = jam_player->position();
        qDebug() << t;
        t = t.addSecs(-10);
        qDebug() << t;
        jam_player->setPosition(t);

    }

}

void MainWindow::loadFile()
{
    QListWidgetItem *curr = songList->currentItem();

    QString loc = MD->getLocation(curr->text());

    float p = MD->getPitch(curr->text());
    float t = MD->getTempo(curr->text());

    if (jam_player->state() == QGst::StatePaused) {
            jam_player->play();
    } else {
        openFile(loc, p, t);
    }
}

void MainWindow::pitchDown()
{
    float pitch = jam_player->getPitch();
    qDebug() << pitch;
    pitch -= 0.05;
    jam_player->setPitch(pitch);
    updatePitchLabel(pitch);

    QListWidgetItem *curr = songList->currentItem();

    QJsonObject details {{"name", curr->text()}, {"field", "pitch"}, {"pitch", pitch}};
    MD->updateSong(details);

}

void MainWindow::pitchUp()
{
    float pitch = jam_player->getPitch();
    qDebug() << pitch;
    pitch += 0.05;
    jam_player->setPitch(pitch);
    updatePitchLabel(pitch);
    QListWidgetItem *curr = songList->currentItem();

    QJsonObject details {{"name", curr->text()}, {"field", "pitch"}, {"pitch", pitch}};
    MD->updateSong(details);

}

void MainWindow::slowDown()
{

    float tempo = jam_player->getTempo();
    tempo -= 0.05;
    jam_player->setTempo(tempo);
    updateTempoLabel(tempo);
    QListWidgetItem *curr = songList->currentItem();

    QJsonObject details {{"name", curr->text()}, {"field", "tempo"}, {"tempo", tempo}};
    MD->updateSong(details);

}

void MainWindow::speedUp()
{
    float tempo = jam_player->getTempo();
    tempo += 0.05;
    jam_player->setTempo(tempo);
    updateTempoLabel(tempo);

    QListWidgetItem *curr = songList->currentItem();

    QJsonObject details {{"name", curr->text()}, {"field", "tempo"}, {"tempo", tempo}};
    MD->updateSong(details);
}

void MainWindow::open()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load a song"), baseDir);

    if (!fileName.isEmpty()) {
        bool ok;
        QString title = "";
        QRegularExpression re("([^\\/]*)$");
        QRegularExpressionMatch match = re.match(fileName);
        if (match.hasMatch()) {
            title = match.captured(1);
        }

        QString text = QInputDialog::getText(this, tr("QInputDialog::getText()"),
                                                 "Please Enter a name for " + fileName, QLineEdit::Normal, title
                                                 , &ok);
        QJsonObject song {{"name", text}, {"location", fileName}, {"pitch", 1.0}, {"tempo", 1.0}};

        MD->addSong(song);

        //new QListWidgetItem(text, songList );
        QJsonArray ja = MD->loadData();
        if ( ja.size() < 1) {
            qDebug() <<"fail";
        }
        updateList(ja);

    }
}

/* Called when the positionChanged() is received from the player */
void MainWindow::onPositionChanged()
{
    QTime length(0,0);
    QTime curpos(0,0);
    const QTime loop(0,1,0);
    const QTime target(0,0,50);

    if (jam_player->state() != QGst::StateReady &&
        jam_player->state() != QGst::StateNull)
    {
        length = jam_player->length();
        curpos = jam_player->position();
    }

    /*
    qDebug() << curpos << " : " << loop << " : " << target;
    if (curpos >= loop) {
        qDebug() << "meep";
        jam_player->setPosition(target);
    }
    */

    positionLabel->setText(curpos.toString("hh:mm:ss.zzz")
                                        + "/" +
                             length.toString("hh:mm:ss.zzz"));

    if (length != QTime(0,0)) {
        positionSlider->setValue(curpos.msecsTo(QTime(0,0)) * 1000 / length.msecsTo(QTime(0,0)));
    } else {
        positionSlider->setValue(0);
    }

    if (curpos != QTime(0,0)) {
        positionLabel->setEnabled(true);
        positionSlider->setEnabled(true);
    }
}

void MainWindow::updatePitchLabel(float pitch)
{
    QVariant p = qRound(pitch * 100);
    pitchLabel->setText("Pitch: " + p.toString() + "%");
}

void MainWindow::updateTempoLabel(float tempo)
{
    QVariant t = qRound(tempo * 100);
    tempoLabel->setText("Tempo: " + t.toString() + "%");
}

void MainWindow::openFile(const QString & fileName, const float pitch, const float tempo)
{
    baseDir = QFileInfo(fileName).path();


    jam_player->stop();
    jam_player->setLocation(fileName);
    jam_player->setPitch(pitch);
    jam_player->setTempo(tempo);

    updatePitchLabel(pitch);
    updateTempoLabel(tempo);

    QTimer::singleShot(5000, this, SLOT(play()));

    //jam_player->play();
}

void MainWindow::play()
{
    jam_player->play();
}


MainWindow::~MainWindow()
{

}
