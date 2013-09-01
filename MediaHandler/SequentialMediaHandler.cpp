#include "SequentialMediaHandler.h"

#include <QAbstractProxyModel>

#include "MediaModel/MediaModel.h"

void SequentialMediaHandler::activate(const QModelIndex& index)
{
    auto mediaModelIndex = this->mediaModelIndex(index);
    auto mediaModel = qobject_cast<const MediaModel*>(mediaModelIndex.model());

    QString title;
    QList<QModelIndex> indexes;
    if (mediaModelIndex.parent() == mediaModel->rootIndex())
    {
        title = mediaModel->fileName(mediaModelIndex);
        indexes.append(mediaModelIndex);
    }
    else
    {
        title = mediaModel->fileName(mediaModelIndex.parent());

        auto item = mediaModelIndex;
        while (item.isValid())
        {
            indexes.append(item);
            item = item.sibling(item.row() + 1, 0);
        }
    }
    this->activateSequence(title, indexes);
}

QModelIndex SequentialMediaHandler::mediaModelIndex(const QModelIndex& index)
{
    if (qobject_cast<const MediaModel*>(index.model()))
    {
        return index;
    }

    auto proxyModel = qobject_cast<const QAbstractProxyModel*>(index.model());
    if (proxyModel)
    {
        return this->mediaModelIndex(proxyModel->mapToSource(index));
    }

    return QModelIndex();
}
