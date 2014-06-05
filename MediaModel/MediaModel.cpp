#include "MediaModel/MediaModel.h"

#include <QFileSystemModel>
#include <QIcon>

#include "MediaDb.h"
#include "MediaModel/Helper/QFileSystemModelWithMappedColumns.h"
#include "Utils.h"

MediaModel::MediaModel(QString rootPath, QObject *parent)
    : QSortFilterProxyModel(parent),
      QFileSystemProxyModelMixin(this)
{
    this->rootPath = rootPath;

    QVector<int> columnMap(4);
    columnMap[0] = 0;
    columnMap[1] = 1;
    columnMap[2] = 1;
    columnMap[3] = 3;
    this->fsModel = new QFileSystemModelWithMappedColumns(columnMap);
    this->fsModel->setRootPath(this->rootPath);
    this->setSourceModel(this->fsModel);
    this->setSourceFileSystemModel(this->fsModel);

    this->setDynamicSortFilter(true);

    connect(&MediaDb::getInstance(), SIGNAL(keyChangedForPath(QString, QString)),
            this, SLOT(onMediaDbKeyChangedForPath(QString, QString)));
}

MediaModel::~MediaModel()
{
    delete this->fsModel;
}

QModelIndex MediaModel::rootIndex() const
{
    return this->mapFromSource(this->fsModel->index(this->fsModel->rootPath()));
}

QVariant MediaModel::data(const QModelIndex& index, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (index.column() == 1)
        {
            QString path = this->filePath(index);

            QString playlistName;
            if (!this->isDir(index))
            {
                if (index.parent().isValid())
                {
                    playlistName = this->filePath(index.parent());
                }
            }

            QString value;
            if (MediaDb::getInstance().contains(path, "progress"))
            {
                value = Utils::formatDuration(MediaDb::getInstance().get(path, "progress").toFloat());
            }
            else if (!playlistName.isEmpty() && MediaDb::getInstance().contains(playlistName, "openingLength"))
            {
                value = Utils::formatDuration(MediaDb::getInstance().get(playlistName, "openingLength").toFloat());
            }
            else
            {
                return "";
            }

            value += " / ";

            if (MediaDb::getInstance().contains(path, "duration"))
            {
                value += Utils::formatDuration(MediaDb::getInstance().get(path, "duration").toFloat());
            }
            else
            {
                value += "--:--";
            }

            return value;
        }

        if (index.column() == 3)
        {
            return this->lastModified(index).toString("dd.MM.yyyy hh:mm");
        }
    }

    if (role == Qt::DecorationRole)
    {
        if (index.column() == 0)
        {
            if (!this->isDir(index))
            {
                return QIcon("://pixel.png");
            }
        }
    }

    if (role == Qt::TextAlignmentRole)
    {
        if (index.column() == 1)
        {
            return Qt::AlignRight;
        }
    }

    if (role == Qt::ForegroundRole)
    {
        QString path = this->filePath(index);
        if (MediaDb::getInstance().contains(path, "duration") && MediaDb::getInstance().contains(path, "progress"))
        {
            if (Utils::isWatched(MediaDb::getInstance().get(path, "progress").toFloat(), MediaDb::getInstance().get(path, "duration").toFloat()))
            {
                return QColor(160, 160, 160);
            }
        }
    }

    return QSortFilterProxyModel::data(index, role);
}

void MediaModel::forceUpdate()
{
    this->fsModel->setRootPath("");
    this->fsModel->setRootPath(this->rootPath);
}

bool MediaModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    QModelIndex sourceIndex = this->fsModel->index(sourceRow, 0, sourceParent);

    // Root
    if (this->fsModel->filePath(sourceIndex) == this->fsModel->rootPath())
    {
        return true;
    }
    // Above root
    if (!this->fsModel->filePath(sourceIndex).contains(this->fsModel->rootPath()))
    {
        return false;
    }
    // Directory
    if (this->fsModel->isDir(sourceIndex))
    {
        return this->anyChildrenIsMovie(sourceIndex) > 0;
    }
    // File
    return this->isMovie(sourceIndex);
}

bool MediaModel::isMovie(const QModelIndex& sourceIndex) const
{
    return Utils::isMovie(this->fsModel->filePath(sourceIndex));
}

bool MediaModel::anyChildrenIsMovie(const QModelIndex& sourceIndex) const
{
    this->fsModel->fetchMore(sourceIndex);
    for (int i = 0; i < this->fsModel->rowCount(sourceIndex); i++)
    {
        QModelIndex sourceChildIndex = this->fsModel->index(i, 0, sourceIndex);
        if (!this->fsModel->isDir(sourceChildIndex))
        {
            if (this->isMovie(sourceChildIndex))
            {
                return true;
            }
        }
    }
    for (int i = 0; i < this->fsModel->rowCount(sourceIndex); i++)
    {
        QModelIndex sourceChildIndex = this->fsModel->index(i, 0, sourceIndex);
        if (this->fsModel->isDir(sourceChildIndex))
        {
            if (this->anyChildrenIsMovie(sourceChildIndex))
            {
                return true;
            }
        }
    }
    return false;
}

void MediaModel::notifyRowUpdate(QString path)
{
    this->notifyRowUpdate(path, 0, this->columnCount());
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

void MediaModel::onMediaDbKeyChangedForPath(QString path, QString key)
{
    Q_UNUSED(key);
    this->notifyRowUpdate(path);
}
