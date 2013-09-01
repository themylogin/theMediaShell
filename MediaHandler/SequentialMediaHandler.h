#ifndef SEQUENTIALMEDIAHANDLER_H
#define SEQUENTIALMEDIAHANDLER_H

#include <QList>
#include <QString>

#include "MediaHandler/MediaHandler.h"

class SequentialMediaHandler : public MediaHandler
{
public:
    void activate(const QModelIndex& index);
protected:
    virtual void activateSequence(const QString& title, const QList<QModelIndex>& indexes) = 0;
private:
    QModelIndex mediaModelIndex(const QModelIndex& index);
};

#endif // SEQUENTIALMEDIAHANDLER_H
