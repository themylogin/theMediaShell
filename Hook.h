#ifndef HOOK_H
#define HOOK_H

#include <QString>

namespace Hook
{
    QString path(const QString& hookName);
    void run(const QString& hookName);
}

#endif // HOOK_H
