#include "MediaModel/MediaModel.h"

#include "MediaConsumptionHistory.h"
#include "MediaModel/Helper/QFileSystemModelWithMappedColumns.h"

MediaModel::MediaModel(QString rootPath, MediaClassificator* classificator, QObject *parent)
    : QSortFilterProxyModel(parent),
      QFileSystemProxyModelMixin(this)
{
    QVector<int> columnMap(3);
    columnMap[0] = 0;
    columnMap[1] = 1;
    columnMap[2] = 3;
    this->fsModel = new QFileSystemModelWithMappedColumns(columnMap);
    this->fsModel->setRootPath(rootPath);
    this->setSourceModel(this->fsModel);
    this->setSourceFileSystemModel(this->fsModel);

    this->classificator = classificator;

    this->setDynamicSortFilter(true);

    connect(&MediaConsumptionHistory::getInstance(), SIGNAL(mediaConsumed(QString, QVariant)),
            this, SLOT(onMediaConsumed(QString, QVariant)));
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
        if (index.column() == 2)
        {
            return this->fsModel->lastModified(this->mapToSource(index)).toString("dd.MM.yyyy hh:mm");
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

    /*
    if (role == Qt::ForegroundRole)
    {
        auto path = this->filePath(index);
        if (MediaConsumptionHistory::getInstance().contains(path))
        {
            float progress = MediaConsumptionHistory::getInstance().getProgress(path);
            float duration = MediaConsumptionHistory::getInstance().getDuration(path);
            int color = progress > duration * 0.5 ? 120 : 80;
            return QColor(color, color, color);
        }
    }*/

    return QSortFilterProxyModel::data(index, role);
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
        return this->anyChildrenIs(sourceIndex) > 0;
    }
    // File
    return this->is(sourceIndex);
}

bool MediaModel::is(const QModelIndex& sourceIndex) const
{
    return this->classificator->is(this->fsModel->filePath(sourceIndex));
}

bool MediaModel::anyChildrenIs(const QModelIndex& sourceIndex) const
{
    this->fsModel->fetchMore(sourceIndex);
    for (int i = 0; i < this->fsModel->rowCount(sourceIndex); i++)
    {
        QModelIndex sourceChildIndex = this->fsModel->index(i, 0, sourceIndex);
        if (!this->fsModel->isDir(sourceChildIndex))
        {
            if (this->is(sourceChildIndex))
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
            if (this->anyChildrenIs(sourceChildIndex))
            {
                return true;
            }
        }
    }
    return false;
}

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

void MediaModel::onMediaConsumed(QString path, QVariant consumeInfo)
{
    Q_UNUSED(consumeInfo);
    this->notifyRowUpdate(path);
}
