#ifndef MEDIAHANDLER_H
#define MEDIAHANDLER_H

#include <QModelIndex>

class MediaHandler
{
public:
    virtual void activate(const QModelIndex& index) = 0;
};

#endif // MEDIAHANDLER_H
