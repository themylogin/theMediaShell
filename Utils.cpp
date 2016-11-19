#include "Utils.h"

#include "X11/Xlib.h"

extern "C"
{
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
}

#include <QGridLayout>
#include <QSpacerItem>
#include <QStringList>
#include <iostream>
#include <QX11Info>

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
                       << ".webm"
                       << ".mpg"
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

    void x11KeyEventForChildren(WId win, bool press, quint32 keysum, quint32 modifiers)
    {
        auto dpy = QX11Info::display();

        Window root;
        Window parent;
        Window* children;
        unsigned int nchildren;
        XQueryTree(dpy, win, &root, &parent, &children, &nchildren);

        for (unsigned int i = 0; i < nchildren; i++)
        {
            XKeyEvent event;
            event.display     = dpy;
            event.window      = children[i];
            event.root        = root;
            event.subwindow   = None;
            event.time        = CurrentTime;
            event.x           = 0;
            event.y           = 0;
            event.x_root      = 0;
            event.y_root      = 0;
            event.same_screen = True;
            event.type        = press ? KeyPress : KeyRelease;
            event.keycode     = XKeysymToKeycode(dpy, keysum);
            event.state       = modifiers;
            XSendEvent(event.display, event.window, True, KeyPressMask, (XEvent*)&event);
        }
    }

    double determineDuration(const QString& path)
    {
        int ret;
        AVFormatContext *formatCtx = NULL;
        double duration = 0;

        av_register_all();

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
