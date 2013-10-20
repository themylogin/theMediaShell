#ifndef MEDIADB_H
#define MEDIADB_H

#include <QDataStream>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QMap>
#include <QString>
#include <QVariant>

class MediaDb : public QObject
{
    Q_OBJECT

public:
    static MediaDb& getInstance()
    {
        static MediaDb db;
        return db;
    }

    bool contains(QString path, QString key)
    {
        return db.contains(path) && db[path].contains(key);
    }

    QVariant get(QString path, QString key)
    {
        return db[path][key];
    }

    void set(QString path, QString key, QVariant value)
    {
        db[path][key] = value;

        this->file.open(QIODevice::WriteOnly);
        QDataStream out(&this->file);
        out << this->db;
        this->file.close();

        emit keyChangedForPath(path, key);
    }

private:
    MediaDb()
    {
        QString appData = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
        QDir dir(appData);
        if (!dir.exists())
        {
            dir.mkpath(appData);
        }

        this->file.setFileName(appData + "/media.db");
        if (this->file.exists())
        {
            this->file.open(QIODevice::ReadOnly);
            QDataStream in(&this->file);
            in >> this->db;
            this->file.close();
        }
    }
    MediaDb(const MediaDb&);
    MediaDb& operator=(const MediaDb&);

    QFile file;
    QMap<QString, QMap<QString, QVariant>> db;

signals:
    void keyChangedForPath(QString path, QString key);
};

#endif // MEDIADB_H
