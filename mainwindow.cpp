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

void MainWindow::updateList(QJsonArray ja)
{
    songList->clear();
    for (int i =0; i < ja.size(); i++) {
        if (ja[i].isObject()) {
            QJsonObject jo = ja[i].toObject();
            qDebug() << jo.value("name").toString();
            new QListWidgetItem(jo.value("name").toString(), songList );
        }
    }

}

void MainWindow::createUI(QBoxLayout *appLayout)
{
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

    connect(jam_player, SIGNAL(positionChanged()), this, SLOT(onPositionChanged()));
    connect(positionSlider, SIGNAL(sliderMoved(int)), this, SLOT(setPosition(int)));

    songList = new QListWidget(this);

    QJsonArray ja = MD->loadData();
    if ( ja.size() < 1) {
        qDebug() <<"fail";
    }
    updateList(ja);

    QGridLayout *posLayout = new QGridLayout;
    posLayout->setContentsMargins(50,50,50,50);
    posLayout->addWidget(songList, 1,1,1,2, Qt::AlignCenter);
    posLayout->addWidget(positionSlider, 2, 1, 1, 2, Qt::AlignVCenter);
    posLayout->addWidget(positionLabel, 3, 1, 1, 2, Qt::AlignHCenter|Qt::AlignTop);
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

    btnLayout->addStretch();

    appLayout->addLayout(btnLayout);


}

void MainWindow::loadFile()
{
    QListWidgetItem *curr = songList->currentItem();

    QString loc = MD->getLocation(curr->text());

    float p = MD->getPitch(curr->text());

    openFile(loc, p);
}

void MainWindow::pitchDown()
{
    float pitch = jam_player->getPitch();
    qDebug() << pitch;
    pitch -= 0.05;
    jam_player->setPitch(pitch);

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

    QListWidgetItem *curr = songList->currentItem();

    QJsonObject details {{"name", curr->text()}, {"field", "pitch"}, {"pitch", pitch}};
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
        QJsonObject song {{"name", text}, {"location", fileName}};

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

    if (jam_player->state() != QGst::StateReady &&
        jam_player->state() != QGst::StateNull)
    {
        length = jam_player->length();
        curpos = jam_player->position();
    }

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


void MainWindow::openFile(const QString & fileName, const float pitch)
{
    baseDir = QFileInfo(fileName).path();


    jam_player->stop();
    jam_player->setLocation(fileName);
    jam_player->setPitch(pitch);
    jam_player->play();
}


MainWindow::~MainWindow()
{

}
