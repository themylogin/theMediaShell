#ifndef PLAYERSESSIONCOUNTDOWN_H
#define PLAYERSESSIONCOUNTDOWN_H

#include <QObject>
#include <QTimer>

class PlayerSessionCountdown : public QObject
{
    Q_OBJECT

public:
    explicit PlayerSessionCountdown(int timeout, QObject* parent = 0);
    void start();
    void stop();

signals:
    void message(QString);
    void timedOut();

private:
    int timeout;
    QTimer* timer;

    int left;

private slots:
    void update();

};

#endif // PLAYERSESSIONCOUNTDOWN_H
