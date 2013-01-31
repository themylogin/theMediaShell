#ifndef NEWMEDIAMODEL_H
#define NEWMEDIAMODEL_H

#include <QDateTime>
#include <QSortFilterProxyModel>

#include "FlatMediaModel.h"
#include "MediaModel.h"

class NewMediaModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    NewMediaModel(MediaModel* mediaModel, QObject *parent = 0)
        : QSortFilterProxyModel(parent)
    {
        this->flatMediaModel = new FlatMediaModel(mediaModel, this);
        this->setSourceModel(this->flatMediaModel);
        this->setDynamicSortFilter(true);
    }

    ~NewMediaModel()
    {
        delete this->flatMediaModel;
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
