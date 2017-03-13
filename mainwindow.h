#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "player.h"
#include "music.h"
#include <QMainWindow>
#include <QTimer>
#include <QWidget>
#include <QStyle>
#include <QObject>
#include <QSlider>
#include <QLabel>
#include <QListWidget>

class QBoxLayout;
class QToolButton;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QListWidget *songList;

private Q_SLOTS:
    void open();
    void loadFile();
    void openFile(const QString & fileName, const float pitch, const float tempo);
    void onStateChanged();
    void onPositionChanged();
    void pitchDown();
    void pitchUp();
    void slowDown();
    void speedUp();

private:
    Player *jam_player;
    Music *MD;
    QToolButton *initButton(QStyle::StandardPixmap icon, const QString & tip,
                                      QObject *dstobj, const char *slot_method, QLayout *layout);
    void createUI(QBoxLayout *appLayout);

    bool loadData();
    void readJson(QJsonObject &json);
    void updateList(QJsonArray);



    QSlider *positionSlider;
    QLabel *positionLabel;

    QString baseDir;
    QToolButton *openButton;
    QToolButton *playButton;
    QToolButton *pauseButton;
    QToolButton *stopButton;
    QToolButton *pitchUpButton;
    QToolButton *pitchDownButton;
    QToolButton *slowDownButton;
    QToolButton *speedUpButton;

};

#endif // MAINWINDOW_H
