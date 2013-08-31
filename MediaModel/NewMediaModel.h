#ifndef NEWMEDIAMODEL_H
#define NEWMEDIAMODEL_H

#include <QDateTime>
#include <QSortFilterProxyModel>

#include "MediaModel/MediaModel.h"
#include "MediaModel/Helper/FlatMediaModel.h"
#include "MediaModel/Helper/QFileSystemProxyModelMixin.h"

class NewMediaModel : public QSortFilterProxyModel, public QFileSystemProxyModelMixin
{
    Q_OBJECT

public:
    NewMediaModel(MediaModel* mediaModel, QObject *parent = 0)
        : QSortFilterProxyModel(parent),
          QFileSystemProxyModelMixin(this)
    {
        this->flatMediaModel = new FlatMediaModel(mediaModel, this);
        this->setSourceModel(this->flatMediaModel);
        this->setSourceFileSystemProxyModel(this->flatMediaModel);
        this->setDynamicSortFilter(true);
    }

    ~NewMediaModel()
    {
        delete this->flatMediaModel;
    }

    QModelIndex mediaModelIndex(const QModelIndex& index)
    {
        return this->flatMediaModel->mapToSource(this->mapToSource(index));
    }

protected:
    bool lessThan(const QModelIndex& sourceLeft, const QModelIndex& sourceRight) const
    {
        return this->flatMediaModel->lastModified(sourceLeft) < this->flatMediaModel->lastModified(sourceRight);
    }

private:
    FlatMediaModel* flatMediaModel;

};

#endif // NEWMEDIAMODEL_H
