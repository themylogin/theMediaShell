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

    float get(QString path)
    {
        QSettings settings;
        return settings.value("MediaConsumptionHistory/" + path).toFloat();
    }

    void set(QString path, float progress)
    {
        QSettings settings;
        settings.setValue("MediaConsumptionHistory/" + path, progress);

        emit mediaConsumed(progress);
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
    void mediaConsumed(float progress);
};

#endif // MEDIACONSUMPTIONHISTORY_H
