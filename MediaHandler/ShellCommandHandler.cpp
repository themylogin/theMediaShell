#include "ShellCommandHandler.h"

#include <QProcess>

ShellCommandHandler::ShellCommandHandler(QString command)
{
    this->command = command;
}

void ShellCommandHandler::activate(const QModelIndex& index)
{
    QProcess* process = new QProcess;
    process->startDetached(this->command.arg(index.data().toString()));
}
