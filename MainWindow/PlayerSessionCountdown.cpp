#include "PlayerSessionCountdown.h"

PlayerSessionCountdown::PlayerSessionCountdown(int timeout, QObject* parent) :
    QObject(parent)
{
    this->timeout = timeout;

    this->timer = new QTimer(this);
    connect(this->timer, SIGNAL(timeout()), this, SLOT(update()));
}

void PlayerSessionCountdown::start()
{
    this->left = this->timeout;
    this->timer->start(1000);
    this->update();
}

void PlayerSessionCountdown::stop()
{
    this->timer->stop();
}

void PlayerSessionCountdown::update()
{
    emit this->message(tr("You will return to your normal activities in: %1").arg(this->left));

    this->left--;
    if (this->left < 0)
    {
        emit this->timedOut();
        this->timer->stop();
    }
}
