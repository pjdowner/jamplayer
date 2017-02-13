#include "sequenceradapter.h"
#include "player.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QBoxLayout>
#include <QFileDialog>
#include <QToolButton>
#include <QLabel>
#include <QSlider>
#include <QMouseEvent>
#include <QShortcut>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    const QString errorstr = "Fatal error from the ALSA sequencer. "
            "This usually happens when the kernel doesn't have ALSA support, "
            "or the device node (/dev/snd/seq) doesn't exists, "
            "or the kernel module (snd_seq) is not loaded. "
            "Please check your ALSA/MIDI configuration.";
    try {
        m_seq = new SequencerAdapter(this);

        //ui->setupUi(this);

        QVariant portName = "24:0";
        m_seq->subscribe(portName.toString());
        m_seq->run();
    }
    catch (const SequencerError& ex) {
        qDebug() << errorstr + " Returned error was: " + ex.qstrError() << endl;
    } catch (...) {
        qDebug() << errorstr << endl;
    }

    tempo = 1.0;
    //create the player
    m_player = new Player(this);

    connect(m_player, SIGNAL(positionChanged()), this, SLOT(onPositionChanged()));
    connect(m_player, SIGNAL(stateChanged()), this, SLOT(onStateChanged()));

    connect(m_seq, SIGNAL(keyPressed(int)), this, SLOT(keyed(int)));

    m_player->hide();

    //m_baseDir is used to remember the last directory that was used.
    //defaults to the current working directory
    m_baseDir = QLatin1String(".");

    QVBoxLayout *appLayout = new QVBoxLayout;
    appLayout->setContentsMargins(0, 0, 0, 0);
    createUI(appLayout);

    QWidget *window = new QWidget();
    window->setLayout(appLayout);

    // Set QWidget as the central layout of the main window
    setCentralWidget(window);

    //setLayout(appLayout);

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

void MainWindow::createUI(QBoxLayout *appLayout)
{
    appLayout->addWidget(m_player);


    m_positionLabel = new QLabel();

    m_positionSlider = new QSlider(Qt::Horizontal);
    m_positionSlider->setTickPosition(QSlider::TicksBelow);
    m_positionSlider->setTickInterval(10);
    m_positionSlider->setMaximum(1000);

    connect(m_positionSlider, SIGNAL(sliderMoved(int)), this, SLOT(setPosition(int)));

    m_volumeSlider = new QSlider(Qt::Horizontal);
    m_volumeSlider->setTickPosition(QSlider::TicksLeft);
    m_volumeSlider->setTickInterval(2);
    m_volumeSlider->setMaximum(10);
    m_volumeSlider->setMaximumSize(64,32);

    connect(m_volumeSlider, SIGNAL(sliderMoved(int)), m_player, SLOT(setVolume(int)));

    QGridLayout *posLayout = new QGridLayout;
    posLayout->addWidget(m_positionLabel, 1, 0);
    posLayout->addWidget(m_positionSlider, 1, 1, 1, 2);
    appLayout->addLayout(posLayout);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->addStretch();

    m_openButton = initButton(QStyle::SP_DialogOpenButton, tr("Open File"),
                              this, SLOT(open()), btnLayout);

    m_playButton = initButton(QStyle::SP_MediaPlay, tr("Play"),
                              m_player, SLOT(play()), btnLayout);

    m_pauseButton = initButton(QStyle::SP_MediaPause, tr("Pause"),
                               m_player, SLOT(pause()), btnLayout);

    m_stopButton = initButton(QStyle::SP_MediaStop, tr("Stop"),
                              m_player, SLOT(stop()), btnLayout);

    m_fullScreenButton = initButton(QStyle::SP_TitleBarMaxButton, tr("Fullscreen"),
                                    this, SLOT(toggleFullScreen()), btnLayout);
    btnLayout->addStretch();

    m_volumeLabel = new QLabel();
    m_volumeLabel->setPixmap(
        style()->standardIcon(QStyle::SP_MediaVolume).pixmap(QSize(32, 32),
                QIcon::Normal, QIcon::On));

    btnLayout->addWidget(m_volumeLabel);
    btnLayout->addWidget(m_volumeSlider);
    appLayout->addLayout(btnLayout);

}

void MainWindow::showControls(bool show)
{
    m_openButton->setVisible(show);
    m_playButton->setVisible(show);
    m_pauseButton->setVisible(show);
    m_stopButton->setVisible(show);
    m_fullScreenButton->setVisible(show);
    m_positionSlider->setVisible(show);
    m_volumeSlider->setVisible(show);
    m_volumeLabel->setVisible(show);
    m_positionLabel->setVisible(show);
}


void MainWindow::toggleFullScreen()
{
    if (isFullScreen()) {
        setMouseTracking(false);
        m_player->setMouseTracking(false);
        m_fullScreenTimer.stop();
        showControls();
        showNormal();
    } else {
        setMouseTracking(true);
        m_player->setMouseTracking(true);
        hideControls();
        showFullScreen();
    }
}

void MainWindow::keyed(int k)
{
    qDebug() << "do something!";
    if (k == 1) {
        tempo = tempo - 0.01;
        m_player->setTempo(tempo);
    } else if (k == 2) {
        tempo = tempo + 0.01;
        m_player->setTempo(tempo);
    }
}

void MainWindow::onStateChanged()
{
    QGst::State newState = m_player->state();
    m_playButton->setEnabled(newState != QGst::StatePlaying);
    m_pauseButton->setEnabled(newState == QGst::StatePlaying);
    m_stopButton->setEnabled(newState != QGst::StateNull);
    m_positionSlider->setEnabled(newState != QGst::StateNull);
    m_volumeSlider->setEnabled(newState != QGst::StateNull);
    m_volumeLabel->setEnabled(newState != QGst::StateNull);
    m_volumeSlider->setValue(m_player->volume());

    //if we are in Null state, call onPositionChanged() to restore
    //the position of the slider and the text on the label
    if (newState == QGst::StateNull) {
        onPositionChanged();
    }
}

/* Called when the positionChanged() is received from the player */
void MainWindow::onPositionChanged()
{
    QTime length(0,0);
    QTime curpos(0,0);

    if (m_player->state() != QGst::StateReady &&
        m_player->state() != QGst::StateNull)
    {
        length = m_player->length();
        curpos = m_player->position();
    }

    m_positionLabel->setText(curpos.toString("hh:mm:ss.zzz")
                                        + "/" +
                             length.toString("hh:mm:ss.zzz"));

    if (length != QTime(0,0)) {
        m_positionSlider->setValue(curpos.msecsTo(QTime(0,0)) * 1000 / length.msecsTo(QTime(0,0)));
    } else {
        m_positionSlider->setValue(0);
    }

    if (curpos != QTime(0,0)) {
        m_positionLabel->setEnabled(true);
        m_positionSlider->setEnabled(true);
    }
}

/* Called when the user changes the slider's position */
void MainWindow::setPosition(int value)
{
    uint length = -m_player->length().msecsTo(QTime(0,0));
    if (length != 0 && value > 0) {
        QTime pos(0,0);
        pos = pos.addMSecs(length * (value / 1000.0));
        m_player->setPosition(pos);
    }
}

void MainWindow::openFile(const QString & fileName)
{
    m_baseDir = QFileInfo(fileName).path();

    m_player->stop();
    m_player->setUri(fileName);
    m_player->play();
}

void MainWindow::open()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open a Movie"), m_baseDir);

    if (!fileName.isEmpty()) {
        openFile(fileName);
    }
}


MainWindow::~MainWindow()
{
    //delete ui;
}
