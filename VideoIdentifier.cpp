#include "VideoIdentifier.h"

#include "string.h"

#include <QFile>
#include <QProcess>
#include <QRegExp>

#include <QtConcurrentRun>

VideoIdentifier& VideoIdentifier::getInstance()
{
    static VideoIdentifier identifier;
    return identifier;
}

bool VideoIdentifier::identify(QString path, VideoIdentification* identification)
{
    if (this->identifications.contains(path))
    {
        auto identificationPair = this->identifications[path];
        if (identificationPair.first == this->pathHash(path))
        {
            if (identificationPair.second.isFinished())
            {
                auto result = identificationPair.second.result();
                memcpy(identification, &result, sizeof(VideoIdentification));
                return true;
            }
            else
            {
                return false;
            }
        }
    }
    else
    {
        auto hash = this->pathHash(path);
        auto future = QtConcurrent::run(this, &VideoIdentifier::doIdentify, path);
        this->identifications[path] = IdentificationPair(hash, future);

        auto watcher = new IdentificationFutureWatcher;
        connect(watcher, SIGNAL(finished()), this, SLOT(watcherFinished()));
        this->identificationsWatchers[watcher] = path;
        watcher->setFuture(future);
    }

    return false;
}

QString VideoIdentifier::pathHash(QString path)
{
    // TODO: include subtitle files around into hash
    return QString("%1").arg(QFile(path).size());
}

VideoIdentification VideoIdentifier::doIdentify(QString path)
{
    QProcess mplayer;
    mplayer.start("mplayer", QStringList() << "-ao" << "null"
                                           << "-vc" << ","
                                           << "-vo" << "null"
                                           << "-frames" << "0"
                                           << "-identify" << path);
    mplayer.waitForFinished(-1);
    QString data = QString::fromUtf8(mplayer.readAllStandardOutput());

    VideoIdentification identification;
    identification.duration = this->duration(data);
    return identification;
}

float VideoIdentifier::duration(const QString &data)
{
    QRegExp rx("ID_LENGTH=([0-9]+)");
    if (rx.lastIndexIn(data) != -1)
    {
        return rx.capturedTexts()[1].toFloat();
    }
    else
    {
        return 0;
    }
}

void VideoIdentifier::watcherFinished()
{
    auto watcher = dynamic_cast<IdentificationFutureWatcher*>(this->sender());
    emit identificationAvailable(this->identificationsWatchers[watcher], watcher->future().result());
    this->identificationsWatchers.remove(watcher);
}
