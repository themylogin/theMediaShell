#ifndef MPDCLIENT_H
#define MPDCLIENT_H

#include <QMutex>
#include <QObject>
#include <QString>

struct MpdState
{
    bool playing;
    int remaining;
    QString artist;
    QString title;
};

class MpdClient : public QObject
{
    Q_OBJECT
public:
    explicit MpdClient(QObject *parent = 0);
    MpdState getState();

signals:
    void stateChanged(MpdState state);

public slots:
    void run();

    void pause();
    void resume();

private:
    MpdState state;
    QMutex stateMutex;
    void setPlaying(bool playing, int remaining = -1, const char* artist = nullptr, const char* title = nullptr);

    bool needsPause;
    bool needsResume;
};

#endif // MPDCLIENT_H
