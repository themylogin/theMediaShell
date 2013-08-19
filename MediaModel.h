#ifndef MEDIAMODEL_H
#define MEDIAMODEL_H

#include <QDateTime>
#include <QRegExp>
#include <QSortFilterProxyModel>

#include "MediaClassificator.h"
#include "MediaConsumptionHistory.h"
#include "QFileSystemModelWithMappedColumns.h"
#include "Utils.h"
#include "VideoIdentifier.h"

class MediaModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    MediaModel(QString rootPath, MediaClassificator* classificator, QObject *parent = 0)
        : QSortFilterProxyModel(parent)
    {
        QVector<int> columnMap(5);
        columnMap[0] = 0;
        columnMap[1] = 1;
        columnMap[2] = 1;
        columnMap[3] = 1;
        columnMap[4] = 3;
        this->fsModel = new QFileSystemModelWithMappedColumns(columnMap);
        this->fsModel->setRootPath(rootPath);
        this->setSourceModel(this->fsModel);

        this->classificator = classificator;

        this->setDynamicSortFilter(true);

        connect(&MediaConsumptionHistory::getInstance(), SIGNAL(mediaConsumed(QString, float, float)),
                this, SLOT(onMediaConsumed(QString, float, float)));
        connect(&VideoIdentifier::getInstance(), SIGNAL(identificationAvailable(QString, VideoIdentification)),
                this, SLOT(onIdentificationAvailable(QString, VideoIdentification)));
    }

    ~MediaModel()
    {
        delete this->fsModel;
    }

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const
    {       
        if (role == Qt::DisplayRole)
        {
            if (index.column() == 1)
            {
                if (this->fsModel->isDir(this->mapToSource(index)))
                {
                    return "";
                }

                QString path = this->fsModel->filePath(this->mapToSource(index));
                if (MediaConsumptionHistory::getInstance().contains(path))
                {
                    return Utils::formatDuration(MediaConsumptionHistory::getInstance().getProgress(path));
                }

                return "";
            }

            if (index.column() == 2)
            {
                if (this->fsModel->isDir(this->mapToSource(index)))
                {
                    return "";
                }

                VideoIdentification id;
                if (this->identification(index, id))
                {
                    return Utils::formatDuration(id.duration);
                }
                else
                {
                    return "...";
                }
            }

            if (index.column() == 4)
            {
                return this->fsModel->lastModified(this->mapToSource(index)).toString("dd.MM.yyyy hh:mm");
            }
        }

        if (role == Qt::TextAlignmentRole)
        {
            if (index.column() == 1 || index.column() == 2 || index.column() == 3)
            {
                return Qt::AlignRight;
            }
            else
            {
                return Qt::AlignLeft;
            }
        }

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

    bool identification(const QModelIndex& index, VideoIdentification& identification) const
    {
        if (this->isDir(index))
        {
            return false;
        }

        return VideoIdentifier::getInstance().identify(this->filePath(index), identification);
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
    QFileSystemModelWithMappedColumns* fsModel;
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

    void notifyRowUpdate(QString path);
    void notifyRowUpdate(QString path, int startColumn, int endColumn);

private slots:
    void onMediaConsumed(QString, float, float);
    void onIdentificationAvailable(QString, VideoIdentification);
};

#endif // MEDIAMODEL_H
