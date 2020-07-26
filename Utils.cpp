#include "Utils.h"

extern "C"
{
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
}

#include <QGridLayout>
#include <QSpacerItem>
#include <QStringList>
#include <iostream>

namespace Utils
{
    bool isMovie(QString path)
    {
        static QStringList extensions;
        if (extensions.isEmpty())
        {
            extensions << ".avi"
                       << ".m4v"
                       << ".mkv"
                       << ".mov"
                       << ".mpg"
                       << ".mp4"
                       << ".ts"
                       << ".webm"
                       << ".wmv";
        }

        foreach (auto extension, extensions)
        {
            if (path.endsWith(extension))
            {
                return true;
            }
        }

        return false;
    }    

    bool isMovieDiscDirectory(QString name)
    {
        static QStringList names;
        if (names.isEmpty())
        {
            names << "BDMV"
                  << "VIDEO_TS";
        }

        return names.contains(name);
    }

    bool isWatched(float progress, float duration)
    {
        return progress / duration > 0.9 || duration - progress < 60;
    }

    QString formatDuration(int duration)
    {
        return QString("%1:%2").arg(duration / 60, 2, 10, QChar('0')).arg(duration % 60, 2, 10, QChar('0'));
    }

    QStringList listSubdirectories(QDir directory)
    {
        QStringList list;
        foreach (QFileInfo info, directory.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot))
        {
            list.append(info.filePath());
            list.append(listSubdirectories(info.filePath()));
        }
        return list;
    }

    bool setStyleSheetFromFile(QWidget* widget, QString fileName)
    {
        QFile qss(fileName);
        if (qss.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            widget->setStyleSheet(qss.readAll());
            qss.close();
            return true;
        }
        return false;
    }

    void resizeMessageBox(QMessageBox* box, int width)
    {
        QSpacerItem* horizontalSpacer = new QSpacerItem(width, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
        QGridLayout* layout = (QGridLayout*)box->layout();
        layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());
    }

    double determineDuration(const QString& path)
    {
        int ret;
        AVFormatContext *formatCtx = NULL;
        double duration = 0;

        auto pathUtf8 = path.toUtf8();
        ret = avformat_open_input(&formatCtx, pathUtf8.constData(), NULL, NULL);
        if (ret != 0)
        {
            goto cleanup;
        }

        // Retrieve stream information
        ret = avformat_find_stream_info(formatCtx, NULL);
        if (ret < 0)
        {
            goto cleanup;
        }

        duration = static_cast<double>(formatCtx->duration) / AV_TIME_BASE;

        cleanup:
        if (formatCtx != NULL)
        {
            avformat_close_input(&formatCtx);
        }

        return duration;
    }
}
