#include "MediaHandler/MovieHandler.h"

#include "MediaModel/Helper/QFileSystemProxyModelMixin.h"
#include "MplayerWindow.h"

void MovieHandler::activate(const QModelIndex& index)
{
    const QFileSystemProxyModelMixin* fsModel = dynamic_cast<const QFileSystemProxyModelMixin*>(index.model());

    QString title;
    QStringList playlist;
    if (!index.parent().isValid() || !index.parent().parent().isValid())
    {
        title = index.data().toString();
        playlist.append(fsModel->filePath(index));
    }
    else
    {
        auto movie = index;
        title = movie.parent().data().toString();
        while (movie.isValid())
        {
            playlist.append(fsModel->filePath(movie));
            movie = movie.sibling(movie.row() + 1, 0);
        }
    }

    MplayerWindow* mplayer = new MplayerWindow(title, playlist);
    Q_UNUSED(mplayer);
}
