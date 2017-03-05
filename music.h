#ifndef MUSIC_H

#include <QIODevice>
#include <QObject>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QDebug>

#define MUSIC_H

class Music : public QObject
{
    Q_OBJECT
public:
    Music(QObject *parent = 0);
    ~Music();

    QJsonArray loadData();
    QString getLocation(QString name);

private:
    QJsonArray ja;

};

#endif // MUSIC_H
