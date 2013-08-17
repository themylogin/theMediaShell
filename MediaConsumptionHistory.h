#ifndef MEDIACONSUMPTIONHISTORY_H
#define MEDIACONSUMPTIONHISTORY_H

#include <QPointF>
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

    float getProgress(QString path)
    {
        QSettings settings;
        return settings.value("MediaConsumptionHistory/" + path).toPointF().x();
    }

    float getDuration(QString path)
    {
        QSettings settings;
        return settings.value("MediaConsumptionHistory/" + path).toPointF().y();
    }

    void set(QString path, float progress, float duration)
    {
        if (!this->contains(path) || this->getProgress(path) < progress)
        {
            QSettings settings;
            settings.setValue("MediaConsumptionHistory/" + path, QPointF(progress, duration));
        }

        emit mediaConsumed(progress, duration);
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
    void mediaConsumed(float progress, float duration);
};

#endif // MEDIACONSUMPTIONHISTORY_H
