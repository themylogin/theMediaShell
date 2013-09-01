#include "MediaHandler/MovieHandler.h"

#include "MediaModel/MediaModel.h"
#include "Movie/MplayerWindow.h"

void MovieHandler::activateSequence(const QString& title, const QList<QModelIndex>& indexes)
{
    QStringList playlist;
    foreach (auto index, indexes)
    {
        auto mediaModel = dynamic_cast<const MediaModel*>(index.model());
        playlist.append(mediaModel->filePath(index));
    }

    MplayerWindow* mplayer = new MplayerWindow(title, playlist);
    Q_UNUSED(mplayer);
}
