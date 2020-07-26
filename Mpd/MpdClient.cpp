#include "MpdClient.h"

#include <unistd.h>

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
        usleep(100000);
    }
}

void MpdClient::pause(bool waitAlsa)
{
    this->needsPause = true;
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
