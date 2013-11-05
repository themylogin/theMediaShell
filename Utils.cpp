#include "Utils.h"

#include <QStringList>

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
                       << ".mp4"
                       << ".ts"
                       << ".webm";
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

    QProcess* runMplayer(const QString& file, const QStringList& userArguments)
    {
        QStringList arguments;

        auto subPaths = listSubdirectories(QFileInfo(file).dir()).join(",");
        if (subPaths != "")
        {
            arguments.append("-sub-paths");
            arguments.append(subPaths);
        }

        QProcess* mplayer = new QProcess;
        mplayer->start("mplayer", QStringList() << arguments
                                                << userArguments
                                                << file);
        return mplayer;
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
}
