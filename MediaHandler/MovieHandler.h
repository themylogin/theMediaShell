#ifndef MOVIEHANDLER_H
#define MOVIEHANDLER_H

#include "MediaHandler/SequentialMediaHandler.h"

class MovieHandler : public SequentialMediaHandler
{
protected:
    void activateSequence(const QString& title, const QList<QModelIndex>& indexes);
};

#endif // MOVIEHANDLER_H
