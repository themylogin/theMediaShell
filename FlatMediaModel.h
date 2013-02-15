#ifndef FLATMEDIAMODEL_H
#define FLATMEDIAMODEL_H

#include <QAbstractProxyModel>
#include <QList>

#include "MediaModel.h"

class FlatMediaModel : public QAbstractProxyModel
{
    Q_OBJECT

public:
    FlatMediaModel(MediaModel* mediaModel, QObject* parent = 0)
        : QAbstractProxyModel(parent)
    {
        this->mediaModel = mediaModel;
        this->setSourceModel(this->mediaModel);
        connect(this->mediaModel, SIGNAL(layoutChanged()), this, SLOT(sourceLayoutChanged()));
    }

    QString filePath(const QModelIndex& index) const
    {
        return this->mediaModel->filePath(this->mapToSource(index));
    }

    QDateTime lastModified(const QModelIndex& index) const
    {
        return this->mediaModel->lastModified(this->mapToSource(index));
    }

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const
    {
        Q_UNUSED(parent);
        return this->createIndex(row, column);
    }

    QModelIndex parent(const QModelIndex&) const
    {
        return QModelIndex();
    }

    int rowCount(const QModelIndex&) const
    {
        return this->flatList.size();
    }

    int columnCount(const QModelIndex& = QModelIndex()) const
    {
        return this->mediaModel->columnCount();
    }

    QModelIndex	mapFromSource(const QModelIndex& sourceIndex) const
    {
        if (!sourceIndex.isValid())
        {
            return QModelIndex();
        }

        QString path = this->mediaModel->filePath(sourceIndex);
        for (int i = 0; i < this->flatList.size(); i++)
        {
            if (path == this->mediaModel->filePath(this->flatList[i]))
            {
                return this->index(i, sourceIndex.column());
            }
        }
        return QModelIndex();
    }

    QModelIndex	mapToSource(const QModelIndex& proxyIndex) const
    {
        if (!proxyIndex.isValid())
        {
            return QModelIndex();
        }
        if (!(proxyIndex.row() < this->flatList.size()))
        {
            return QModelIndex();
        }

        return this->mediaModel->index(this->flatList[proxyIndex.row()].row(), proxyIndex.column(), this->flatList[proxyIndex.row()].parent());
    }

private:
    MediaModel* mediaModel;
    QList<QModelIndex> flatList;

    QList<QModelIndex> buildFlatList(const QModelIndex& sourceIndex) const
    {
        QList<QModelIndex> list;
        this->mediaModel->fetchMore(sourceIndex);
        for (int i = 0; i < this->mediaModel->rowCount(sourceIndex); i++)
        {
            QModelIndex sourceChildIndex = this->mediaModel->index(i, 0, sourceIndex);
            if (this->mediaModel->isDir(sourceChildIndex))
            {
                list.append(this->buildFlatList(sourceChildIndex));
            }
            else
            {
                list.append(sourceChildIndex);
            }
        }
        return list;
    }

private slots:
    void sourceLayoutChanged()
    {
        this->flatList = this->buildFlatList(this->mediaModel->rootIndex());
        this->reset();
    }
};

#endif // FLATMEDIAMODEL_H
