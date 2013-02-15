#ifndef MEDIACONSUMPTIONHISTORY_H
#define MEDIACONSUMPTIONHISTORY_H

#include <QSettings>

class MediaConsumptionHistory
{
public:
    void set(QString path, QVariant data)
    {
        QSettings settings;
        settings.setValue("MediaConsumptionHistory/" + path, data);
    }

    bool contains(QString path)
    {
        QSettings settings;
        return settings.contains("MediaConsumptionHistory/" + path);
    }
};

#endif // MEDIACONSUMPTIONHISTORY_H
