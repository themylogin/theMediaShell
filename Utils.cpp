#include "Utils.h"

namespace Utils
{
    QString formatDuration(int duration)
    {
        return QString("%1:%2").arg(duration / 60, 2, 10, QChar('0')).arg(duration % 60, 2, 10, QChar('0'));
    }
}
