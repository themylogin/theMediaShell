#ifndef MEDIAMODEL_H
#define MEDIAMODEL_H

#include <QDateTime>
#include <QFileSystemModel>
#include <QSortFilterProxyModel>

#include "MediaClassificator.h"
#include "MediaConsumptionHistory.h"

class MediaModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    MediaModel(QString rootPath, MediaClassificator* classificator, QObject *parent = 0)
        : QSortFilterProxyModel(parent)
    {
        this->fsModel = new QFileSystemModel;
        this->fsModel->setRootPath(rootPath);
        this->setSourceModel(this->fsModel);

        this->classificator = classificator;

        this->setDynamicSortFilter(true);
    }

    ~MediaModel()
    {
        delete this->fsModel;
    }

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const
    {
        if (role == Qt::DisplayRole)
        {
            if (index.column() == 3)
            {
                return this->fsModel->lastModified(this->mapToSource(index)).toString("dd.MM.yyyy hh:mm");
            }
        }

        if (role == Qt::TextAlignmentRole)
        {
            if (index.column() == 1)
            {
                return Qt::AlignRight; // size
            }
            else
            {
                return Qt::AlignLeft;
            }
        }

        /*
        // Reuse "checked state" as "watched state"
        // As stated in http://stackoverflow.com/questions/14642254/styling-qt-model-view-items-based-on-their-itemdatarole-properties, this is the only way to style it via stylesheet
        if (role == Qt::CheckStateRole)
        {
            MediaConsumptionHistory history;
            if (history.contains(this->filePath(index)))
            {
                return Qt::Checked;
            }
        }
        */
        // HACK: QTreeView::item:checked does not work, this is temporary solution
        if (role == Qt::ForegroundRole)
        {
            MediaConsumptionHistory history;
            if (history.contains(this->filePath(index)))
            {
                return QColor(120, 120, 120);
            }
        }

        return QSortFilterProxyModel::data(index, role);
    }

    QModelIndex rootIndex() const
    {
        return this->mapFromSource(this->fsModel->index(this->fsModel->rootPath()));
    }

    bool isDir(const QModelIndex& index) const
    {
        return this->fsModel->isDir(this->mapToSource(index));
    }

    QString filePath(const QModelIndex& index) const
    {
        return this->fsModel->filePath(this->mapToSource(index));
    }

    QDateTime lastModified(const QModelIndex& index) const
    {
        return this->fsModel->lastModified(this->mapToSource(index));
    }

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
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

private:
    QFileSystemModel* fsModel;
    MediaClassificator* classificator;

    bool is(const QModelIndex& sourceIndex) const
    {
        return this->classificator->is(this->fsModel->filePath(sourceIndex));
    }

    bool anyChildrenIs(const QModelIndex& sourceIndex) const
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
};

#endif // MEDIAMODEL_H
