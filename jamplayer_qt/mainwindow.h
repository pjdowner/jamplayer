#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QWidget>
#include <QStyle>

#include <QObject>
#include <QReadWriteLock>

class SequencerAdapter;
class Player;
class QBoxLayout;
class QLabel;
class QSlider;
class QToolButton;
class QTimer;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void openFile(const QString & fileName);

private Q_SLOTS:
    void open();
    void toggleFullScreen();

    void onStateChanged();
    void onPositionChanged();
    void keyed(int k);

    void setPosition(int position);

    void showControls(bool show = true);
    void hideControls() { showControls(false); }

private:
    QToolButton *initButton(QStyle::StandardPixmap icon, const QString & tip,
                            QObject *dstobj, const char *slot_method, QLayout *layout);
    Ui::MainWindow *ui;
    SequencerAdapter* m_seq;
    Player *m_player;
    float tempo;


    void createUI(QBoxLayout *appLayout);

    QString m_baseDir;
    QToolButton *m_openButton;
    QToolButton *m_fullScreenButton;
    QToolButton *m_playButton;
    QToolButton *m_pauseButton;
    QToolButton *m_stopButton;
    QToolButton *m_pitchDown;
    QToolButton *m_pitchUp;

    QSlider *m_positionSlider;
    QSlider *m_volumeSlider;
    QLabel *m_positionLabel;
    QLabel *m_volumeLabel;
    QTimer m_fullScreenTimer;
};

#endif // MAINWINDOW_H
