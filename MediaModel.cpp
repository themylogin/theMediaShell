#include "MediaModel.h"

void MediaModel::notifyRowUpdate(QString path)
{
    this->notifyRowUpdate(path, 0, 4);
}

void MediaModel::notifyRowUpdate(QString path, int startColumn, int endColumn)
{
    QModelIndex index = this->mapFromSource(this->fsModel->index(path));
    if (index.isValid())
    {
        QModelIndex topLeft = this->index(index.row(), startColumn, index.parent());
        QModelIndex bottomRight = this->index(index.row(), endColumn, index.parent());
        emit dataChanged(topLeft, bottomRight);
    }
}

void MediaModel::onMediaConsumed(QString path, float progress, float duration)
{
    Q_UNUSED(progress);
    Q_UNUSED(duration);
    this->notifyRowUpdate(path);
}

void MediaModel::onIdentificationAvailable(QString path, VideoIdentification)
{
    this->notifyRowUpdate(path, 2, 3);
}
