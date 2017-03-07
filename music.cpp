#include "music.h"

Music::Music (QObject *parent) :
        QObject(parent)
{

}


Music::~Music()
{

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
