#ifndef SHELLCOMMANDHANDLER_H
#define SHELLCOMMANDHANDLER_H

#include <QString>

#include "MediaHandler/MediaHandler.h"

class ShellCommandHandler : public MediaHandler
{
public:
    ShellCommandHandler(QString command);
    void activate(const QModelIndex& index);
private:
    QString command;
};

#endif // SHELLCOMMANDHANDLER_H
