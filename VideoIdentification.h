#ifndef VIDEOIDENTIFICATION_H
#define VIDEOIDENTIFICATION_H

#include <QStringList>

class VideoIdentification
{
public:
    float duration;

    QStringList subtitles;
    QStringList abandonedSubtitles;
};

#endif // VIDEOIDENTIFICATION_H
