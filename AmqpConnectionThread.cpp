#include "AmqpConnectionThread.h"

#include <iostream>

#include <SimpleAmqpClient/SimpleAmqpClient.h>
#include <qjson/parser.h>

using namespace AmqpClient;

AmqpConnectionThread::AmqpConnectionThread(QString broker, QObject* parent) :
    QThread(parent)
{
    this->broker = broker;
}

void AmqpConnectionThread::run()
{
    while (true)
    {
        try
        {
            auto channel = Channel::Create(this->broker.toLocal8Bit().constData());
            channel->DeclareExchange("smarthome_events", Channel::EXCHANGE_TYPE_TOPIC);

            auto queue = channel->DeclareQueue("");
            channel->BindQueue(queue, "smarthome_events", "bathroom_light.on changed");

            auto consumerTag = "theMediaShell";
            channel->BasicConsume(queue, consumerTag);
            while (true)
            {
                Envelope::ptr_t env;
                if (channel->BasicConsumeMessage(consumerTag, env, 0))
                {
                    QJson::Parser parser;
                    QVariantMap body = parser.parse(env->Message()->Body().c_str()).toMap();

                    emit messageReceived(env->RoutingKey().c_str(), body);
                }
            }
        }
        catch (std::exception e)
        {
            std::cerr << "catched exception in AmqpConnectionThread: " << e.what() << std::endl;
            this->sleep(1);
        }
    }
}
