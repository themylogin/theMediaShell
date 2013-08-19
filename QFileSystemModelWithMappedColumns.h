#ifndef QFILESYSTEMMODELWITHMAPPEDCOLUMNS_H
#define QFILESYSTEMMODELWITHMAPPEDCOLUMNS_H

#include <QFileSystemModel>
#include <QVector>

class QFileSystemModelWithMappedColumns : public QFileSystemModel
{
    Q_OBJECT

public:
    QFileSystemModelWithMappedColumns(QVector<int> columnMap)
        : QFileSystemModel()
    {
        this->columnMap = columnMap;
    }

    int columnCount(const QModelIndex& parent) const
    {
        return parent.column() > 0 ? 0 : this->columnMap.count();
    }

    Qt::ItemFlags flags(const QModelIndex& index) const
    {
        return QFileSystemModel::flags(this->mapIndex(index));
    }

    QVariant data(const QModelIndex& index, int role) const
    {
        return QFileSystemModel::data(this->mapIndex(index), role);
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role) const
    {
        return QFileSystemModel::headerData(this->columnMap[section], orientation, role);
    }

private:
    QVector<int> columnMap;

    QModelIndex mapIndex(const QModelIndex& index) const
    {
        return this->index(index.row(), this->columnMap[index.column()], index.parent());
    }
};

#endif // QFILESYSTEMMODELWITHMAPPEDCOLUMNS_H
