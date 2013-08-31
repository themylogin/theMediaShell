#ifndef MOVIEHANDLER_H
#define MOVIEHANDLER_H

#include "MediaHandler/MediaHandler.h"

class MovieHandler : public MediaHandler
{
public:
    void activate(const QModelIndex& index);
};

#endif // MOVIEHANDLER_H
