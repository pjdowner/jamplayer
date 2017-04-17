/*
Copyright (C) 2017 Philip Downer <phil@pjd.me.uk>

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
    float getPitch(QString name);
    float getTempo(QString name);
    void addSong(QJsonObject song);
    void updateSong(QJsonObject details);

private:
    QJsonArray ja;

};

#endif // MUSIC_H
