#ifndef MEDIACONSUMPTIONHISTORY_H
#define MEDIACONSUMPTIONHISTORY_H

#include <QSettings>

class MediaConsumptionHistory : public QObject
{
    Q_OBJECT

public:
    static MediaConsumptionHistory& getInstance()
    {
        static MediaConsumptionHistory history;
        return history;
    }

    void set(QString path, QVariant data)
    {
        QSettings settings;
        settings.setValue("MediaConsumptionHistory/" + path, data);

        emit mediaConsumed(data);
    }

    bool contains(QString path)
    {
        QSettings settings;
        return settings.contains("MediaConsumptionHistory/" + path);
    }

private:
    MediaConsumptionHistory(){}
    MediaConsumptionHistory(const MediaConsumptionHistory&);
    MediaConsumptionHistory& operator=(const MediaConsumptionHistory&);

signals:
    void mediaConsumed(QVariant data);
};

#endif // MEDIACONSUMPTIONHISTORY_H
