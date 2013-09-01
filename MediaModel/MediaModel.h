#ifndef MEDIAMODEL_H
#define MEDIAMODEL_H

#include <QIcon>
#include <QSortFilterProxyModel>

#include "Classificator/MediaClassificator.h"
#include "MediaModel/Helper/QFileSystemProxyModelMixin.h"

class MediaModel : public QSortFilterProxyModel, public QFileSystemProxyModelMixin
{
    Q_OBJECT

public:
    MediaModel(QString rootPath, MediaClassificator* classificator, QObject *parent = 0);
    ~MediaModel();

    QModelIndex rootIndex() const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const;

private:
    QFileSystemModel* fsModel;
    MediaClassificator* classificator;

    bool is(const QModelIndex& sourceIndex) const;
    bool anyChildrenIs(const QModelIndex& sourceIndex) const;

    void notifyRowUpdate(QString path);
    void notifyRowUpdate(QString path, int startColumn, int endColumn);

private slots:
    void onMediaConsumed(QString, QVariant);
};

#endif // MEDIAMODEL_H
