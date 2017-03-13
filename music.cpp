#include "music.h"

Music::Music (QObject *parent) :
        QObject(parent)
{

}


Music::~Music()
{

}

float Music::getTempo(QString name){
    qDebug() << "search: " << name;

    for (int i =0; i < ja.size(); i++) {
        if (ja[i].isObject()) {
            QJsonObject jo = ja[i].toObject();
            if (QString::compare( jo.value("name").toString(), name, Qt::CaseInsensitive) == 0){
                qDebug() << "getpitch " << jo.value("tempo");

                return jo.value("tempo").toDouble();
            }
        }
    }
    return(1.0);
}

float Music::getPitch(QString name){
    qDebug() << "search: " << name;

    for (int i =0; i < ja.size(); i++) {
        if (ja[i].isObject()) {
            QJsonObject jo = ja[i].toObject();
            if (QString::compare( jo.value("name").toString(), name, Qt::CaseInsensitive) == 0){
                qDebug() << "getpitch " << jo.value("pitch");

                return jo.value("pitch").toDouble();
            }
        }
    }
    return(1.0);
}

QString Music::getLocation(QString name){
    qDebug() << "search: " << name;

    for (int i =0; i < ja.size(); i++) {
        if (ja[i].isObject()) {
            QJsonObject jo = ja[i].toObject();
            if (QString::compare( jo.value("name").toString(), name, Qt::CaseInsensitive) == 0){
                qDebug() << jo.value("location");
                return jo.value("location").toString();
            }
        }
    }
    return("");
}

void Music::addSong(QJsonObject song)
{
    QFile d("/home/phil/songs.json");
    if (!d.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open file.");
    } else {

        ja.append(song);
        QJsonDocument jd(ja);
        d.write(jd.toJson());
        d.close();
    }
}

void Music::updateSong(QJsonObject details)
{

    QFile d("/home/phil/songs.json");
    if (!d.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open file.");
    } else {
        for (int i =0; i < ja.size(); i++) {
            if (ja[i].isObject()) {
                QJsonObject jo = ja[i].toObject();
                if (QString::compare( jo.value("name").toString(), details.value("name").toString(), Qt::CaseInsensitive) == 0){

                    QString field = details.value("field").toString();
                    QJsonValue val = details.value(field);

                    //if (!jo.contains(field)) {
                        jo.insert(details.value("field").toString(), val);

                    //} else {
                    //    jo.value(field) = val;
                    //}

                    ja[i] = jo;
                    QJsonDocument jd(ja);
                    d.write(jd.toJson());
                    d.close();
                    return;
                }
            }
        }
    }
}

QJsonArray Music::loadData()
{
    QFile d("/home/phil/songs.json");


    if (!d.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open file.");
               return ja;
    }

    QByteArray data = d.readAll();

    QJsonDocument jd = QJsonDocument::fromJson(data);
    ja = jd.array();


    return ja;

}
