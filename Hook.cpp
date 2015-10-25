#include "Hook.h"

#include <QDir>
#include <QFileInfo>
#include <QProcess>

#include <qapplication.h>

namespace Hook
{
    QString path(const QString& hookName)
    {
        return QFileInfo(qApp->arguments()[0]).absoluteDir().absolutePath() + "/hooks/" + hookName;
    }

    void run(const QString& hookName)
    {
        QProcess hook;
        hook.start(path(hookName));
        hook.waitForFinished();
    }
}
