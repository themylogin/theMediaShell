#include "MpdClient.h"

#include <unistd.h>

#include <alsa/asoundlib.h>
#include <mpd/client.h>

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QMessageBox>
#include <QMetaType>

#include "Utils.h"

MpdClient::MpdClient(QObject *parent) :
    QObject(parent)
{    
    qRegisterMetaType<MpdState>("MpdState");

    this->state.playing = false;

    this->needsPause = false;
    this->needsResume = false;
}

MpdState MpdClient::getState()
{
    QMutexLocker ml(&this->stateMutex);
    return this->state;
}

void MpdClient::run()
{
    while (1)
    {
        struct mpd_connection* conn = mpd_connection_new(NULL, 0, 30000);
        if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS)
        {
            goto finish;
        }

        while (1)
        {
            if (this->needsPause)
            {
                if (!mpd_run_pause(conn, true))
                {
                    goto finish;
                }
                this->needsPause = false;
            }

            if (this->needsResume)
            {
                if (!mpd_run_pause(conn, false))
                {
                    goto finish;
                }
                this->needsResume = false;
            }

            struct mpd_status* status = mpd_run_status(conn);
            if (status == NULL)
            {
                goto finish;
            }
            if (mpd_status_get_state(status) == MPD_STATE_PLAY)
            {
                struct mpd_song* song = mpd_run_get_queue_song_id(conn, mpd_status_get_song_id(status));
                if (song == NULL)
                {
                    goto finish;
                }

                this->setPlaying(true,
                                 mpd_status_get_total_time(status) - mpd_status_get_elapsed_time(status),
                                 mpd_song_get_tag(song, MPD_TAG_ARTIST, 0),
                                 mpd_song_get_tag(song, MPD_TAG_TITLE, 0));
            }
            else
            {
                this->setPlaying(false);
            }

            if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS)
            {
                goto finish;
            }

            usleep(100000);
        }

        finish:
        this->setPlaying(false);
        mpd_connection_free(conn);
        usleep(100000);
    }
}

void MpdClient::pause(bool waitAlsa)
{
    this->needsPause = true;

    if (waitAlsa)
    {
        // Wait for our slow computer to free ALSA device
        QElapsedTimer timer;
        timer.start();
        while (true)
        {
            QCoreApplication::processEvents();

            int err;
            snd_pcm_t* handle;
            if ((err = snd_pcm_open(&handle, "plughw:0,0", SND_PCM_STREAM_PLAYBACK, 0)) < 0)
            {
                if (timer.elapsed() > 5000)
                {
                    QMessageBox errorBox;
                    Utils::setStyleSheetFromFile(&errorBox, "://QMessageBox.qss");
                    errorBox.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
                    errorBox.setText(QString("Unable to open ALSA device: %1").arg(snd_strerror(err)));
                    Utils::resizeMessageBox(&errorBox);
                    errorBox.exec();
                    return;
                }
            }
            else
            {
                snd_pcm_close(handle);
                return;
            }
        }
    }
}

void MpdClient::resume()
{
    this->needsResume = true;
}

void MpdClient::setPlaying(bool playing, int remaining, const char* artist, const char* title)
{
    QMutexLocker ml(&this->stateMutex);

    if (playing)
    {
        this->state.playing = true;
        this->state.remaining = remaining;
        this->state.artist = artist ? QString::fromUtf8(artist) : QString();
        this->state.title = title ? QString::fromUtf8(title) : QString();
        emit stateChanged(this->state);
    }
    else
    {
        if (this->state.playing)
        {
            this->state.playing = false;
            emit stateChanged(this->state);
        }
    }
}
