#ifndef AMQPCONNECTIONTHREAD_H
#define AMQPCONNECTIONTHREAD_H

#include <QString>
#include <QThread>
#include <QVariantMap>

class AmqpConnectionThread : public QThread
{
    Q_OBJECT

public:
    explicit AmqpConnectionThread(QString broker, QObject* parent = 0);
    void run();
    
signals:
    void messageReceived(QString name, QVariantMap body);

private:
    QString broker;
};

#endif // AMQPCONNECTIONTHREAD_H
