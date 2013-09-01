#include "VideoIdentifier.h"

#include "string.h"

#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QRegExp>
#include <QSet>

#include <QtConcurrentRun>

#include "Classificator/Classificators.h"
#include "Utils.h"

VideoIdentifier& VideoIdentifier::getInstance()
{
    static VideoIdentifier identifier;
    return identifier;
}

bool VideoIdentifier::identify(QString path, VideoIdentification& identification)
{
    if (this->identifications.contains(path))
    {
        auto identificationPair = this->identifications[path];
        if (identificationPair.first == this->pathHash(path))
        {
            if (identificationPair.second.isFinished())
            {
                identification = identificationPair.second.result();
                return true;
            }
            else
            {
                return false;
            }
        }
    }

    auto hash = this->pathHash(path);
    auto future = QtConcurrent::run(this, &VideoIdentifier::doIdentify, path);
    this->identifications[path] = IdentificationPair(hash, future);

    auto watcher = new IdentificationFutureWatcher;
    connect(watcher, SIGNAL(finished()), this, SLOT(watcherFinished()));
    this->identificationsWatchers[watcher] = path;
    watcher->setFuture(future);

    return false;
}

QString VideoIdentifier::pathHash(const QString& path)
{
    // TODO: include subtitle files around into hash
    return QString("%1").arg(QFile(path).size());
}

VideoIdentification VideoIdentifier::doIdentify(const QString& path)
{
    QProcess* mplayer = Utils::runMplayer(path, QStringList() << "-ao" << "null"
                                                              << "-vc" << ","
                                                              << "-vo" << "null"
                                                              << "-frames" << "0"
                                                              << "-identify");
    mplayer->waitForFinished(-1);
    QString data = QString::fromUtf8(mplayer->readAllStandardOutput());
    delete mplayer;

    VideoIdentification identification;
    identification.duration = this->duration(data);
    identification.subtitles = this->listSubtitles(data);
    identification.abandonedSubtitles = this->listAbandonedSubtitles(path);
    return identification;
}

float VideoIdentifier::duration(const QString& data)
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

QStringList VideoIdentifier::listSubtitles(const QString& data)
{
    int pos;
    QStringList subtitles;

    pos = 0;
    QRegExp internalSubtitleRx("ID_SID_[0-9]+_LANG=([^\n]+)");
    while ((pos = internalSubtitleRx.indexIn(data, pos)) != -1)
    {
        subtitles.append(internalSubtitleRx.cap(1));
        pos += internalSubtitleRx.matchedLength();
    }

    QRegExp filenameRx("ID_FILENAME=([^\n]+)");
    if (filenameRx.indexIn(data) == -1)
    {
        return subtitles;
    }
    QFileInfo fileInfo(filenameRx.capturedTexts()[1]);

    pos = 0;
    QRegExp externalSubtitleRx("ID_FILE_SUB_FILENAME=([^\n]+)");
    while ((pos = externalSubtitleRx.indexIn(data, pos)) != -1)
    {
        QFileInfo externalSubtitleInfo(QFile(externalSubtitleRx.cap(1)));

        QString directory = externalSubtitleInfo.absolutePath().mid(fileInfo.absolutePath().length() + 1);
        QString subtitleName = externalSubtitleInfo.fileName().replace(fileInfo.completeBaseName(), "") + (directory != "" ? " (" + directory + ")" : "");

        subtitles.append(subtitleName);
        pos += externalSubtitleRx.matchedLength();
    }

    return subtitles;
}

QStringList VideoIdentifier::listAbandonedSubtitles(const QString& path)
{
    auto fileDirectory = QFileInfo(path).dir();

    QList<QString> videoFileBaseNames;
    foreach (QFileInfo info, QDir(fileDirectory).entryInfoList(QDir::Files))
    {
        if (videoClassificator->is(info.fileName()))
        {
            videoFileBaseNames.append(info.completeBaseName());
        }
    }

    QStringList abandonedSubtitles;
    foreach (QString directory, QStringList() << fileDirectory.path() << Utils::listSubdirectories(fileDirectory))
    {
        foreach (QFileInfo info, QDir(directory).entryInfoList(QDir::Files))
        {
            QString subtitleFilename = info.fileName();
            if (subtitleClassificator->is(subtitleFilename))
            {
                bool found = false;
                foreach (QString videoFileBaseName, videoFileBaseNames)
                {
                    if (subtitleFilename.startsWith(videoFileBaseName))
                    {
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    abandonedSubtitles.append(info.absoluteFilePath().mid(fileDirectory.absolutePath().length() + 1));
                }
            }
        }
    }

    return abandonedSubtitles;
}

#include <QDebug>

void VideoIdentifier::watcherFinished()
{
    auto watcher = dynamic_cast<IdentificationFutureWatcher*>(this->sender());
    emit identificationAvailable(this->identificationsWatchers[watcher], watcher->future().result());
    this->identificationsWatchers.remove(watcher);
}
