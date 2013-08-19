#ifndef VIDEOIDENTIFIER_H
#define VIDEOIDENTIFIER_H

#include <QDir>
#include <QFuture>
#include <QFutureWatcher>
#include <QMap>
#include <QObject>
#include <QString>

#include "VideoIdentification.h"

class VideoIdentifier : public QObject
{
    Q_OBJECT

public:
    static VideoIdentifier& getInstance();
    bool identify(QString path, VideoIdentification& identification);
    
signals:
    void identificationAvailable(QString, VideoIdentification);
    
private:
    typedef QPair<QString, QFuture<VideoIdentification>> IdentificationPair;
    typedef QFutureWatcher<VideoIdentification> IdentificationFutureWatcher;

    QMap<QString, IdentificationPair> identifications;
    QMap<IdentificationFutureWatcher*, QString> identificationsWatchers;

    VideoIdentifier() : QObject(0){}
    VideoIdentifier(const VideoIdentifier&);
    VideoIdentifier& operator=(const VideoIdentifier&);

    QString pathHash(const QString& path);
    VideoIdentification doIdentify(const QString& path);

    float duration(const QString& data);
    QStringList listSubtitles(const QString& data);
    QStringList listAbandonedSubtitles(const QString& path);

private slots:
    void watcherFinished();
};

#endif // VIDEOIDENTIFIER_H