#ifndef PLAYLISTMODEL_H
#define PLAYLISTMODEL_H

#include <cmath>

#include <QAbstractTableModel>
#include <QDateTime>
#include <QList>
#include <QStringList>

#include "Player/PlaylistItem.h"

class PlaylistModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    PlaylistModel(QObject *parent = 0)
        : QAbstractTableModel(parent)
    {
    }

    int rowCount(const QModelIndex& parent = QModelIndex()) const
    {
        Q_UNUSED(parent);

        return this->playlist.size();
    }

    int columnCount(const QModelIndex& parent = QModelIndex()) const
    {
        Q_UNUSED(parent);

        return 2;
    }

    QVariant data(const QModelIndex& index, int role) const
    {
        if (role == Qt::TextAlignmentRole && index.column() == 1)
        {
            return Qt::AlignRight;
        }
        if (role != Qt::DisplayRole)
        {
            return QVariant();
        }
        if (!index.isValid())
        {
            return QVariant();
        }
        if (index.row() >= this->playlist.size())
        {
            return QVariant();
        }

        PlaylistItem* item = this->playlist[index.row()];
        if (index.column() == 0)
        {
            return QVariant(item->title);
        }
        if (index.column() == 1)
        {
            QDateTime willEndAt = this->firstItemWillEndAt;
            if (willEndAt.isNull())
            {
                return QVariant("...");
            }
            for (int i = 1; i <= index.row(); i++)
            {
                float duration = this->playlist[index.row()]->duration;
                if (std::isnan(duration))
                {
                    return QVariant("...");
                }
                willEndAt = willEndAt.addMSecs(duration * 1000);
            }
            return QVariant(willEndAt.toString("hh:mm"));
        }
        return QVariant();
    }

    Qt::ItemFlags flags(const QModelIndex& index) const
    {
        if (!index.isValid())
        {
            return Qt::NoItemFlags;
        }
        if (index.row() >= this->playlist.size())
        {
            return Qt::NoItemFlags;
        }

        if (this->playlist[index.row()]->isActive)
        {
            return Qt::ItemIsEnabled;
        }
        else
        {
            return Qt::NoItemFlags;
        }
    }

    void addItem(PlaylistItem* item)
    {
        this->beginInsertRows(QModelIndex(), this->playlist.count(), this->playlist.count() + 1);
        this->playlist.push_back(item);
        this->endInsertRows();
    }

    bool setDurationFor(QString file, float duration)
    {
        foreach (PlaylistItem* item, this->playlist)
        {
            if (item->file == file)
            {
                item->duration = duration;
                emit dataChanged(this->createIndex(this->playlist.indexOf(item), 1), this->createIndex(this->playlist.size() - 1, 1));
                return true;
            }
        }

        return false;
    }

    int activeCount()
    {
        for (int i = 0; i < this->playlist.count(); i++)
        {
            if (!this->playlist[i]->isActive)
            {
                return i;
            }
        }
        return this->playlist.count();
    }

    void setActiveCount(int count)
    {
        foreach (PlaylistItem* item, this->playlist)
        {
            item->isActive = false;
        }
        for (int i = 0; i < count && i < this->playlist.count(); i++)
        {
            this->playlist[i]->isActive = true;
        }

        emit dataChanged(this->createIndex(0, 0), this->createIndex(this->rowCount() - 1, this->columnCount() - 1)); // TODO: We can provide more accurate information
    }

    void notify(const QDateTime& currentDateTime, double firstItemRemaining)
    {
        QDateTime newFirstItemWillEndAt = currentDateTime.addMSecs(firstItemRemaining * 1000);
        if (this->firstItemWillEndAt.isNull() ||
            abs(newFirstItemWillEndAt.toMSecsSinceEpoch() - this->firstItemWillEndAt.toMSecsSinceEpoch()) > 1000)
        {
            this->firstItemWillEndAt = newFirstItemWillEndAt;
            emit dataChanged(this->createIndex(0, 1), this->createIndex(this->playlist.size() - 1, 1));
        }
    }

    PlaylistItem* getFrontItem() const
    {
        if (this->playlist.count())
        {
            return this->playlist[0];
        }
        return 0;
    }

    void popFrontItem()
    {
        this->beginRemoveRows(QModelIndex(), 0, 0);
        this->playlist.pop_front();
        this->endRemoveRows();
    }

private:
    QList<PlaylistItem*> playlist;
    QDateTime firstItemWillEndAt;
};

#endif // PLAYLISTMODEL_H
