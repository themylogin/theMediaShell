#ifndef PLAYLISTITEM_H
#define PLAYLISTITEM_H

#include <QString>

struct PlaylistItem
{
    QString file;
    float duration;
    bool isActive;
};

#endif // PLAYLISTITEM_H
