#ifndef MEDIAMODEL_H
#define MEDIAMODEL_H

#include <QSortFilterProxyModel>

#include "MediaModel/Helper/QFileSystemProxyModelMixin.h"

class MediaModel : public QSortFilterProxyModel, public QFileSystemProxyModelMixin
{
    Q_OBJECT

public:
    MediaModel(QString rootPath, QObject *parent = 0);
    ~MediaModel();

    QModelIndex rootIndex() const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    void forceUpdate();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const;

private:
    QString rootPath;
    QFileSystemModel* fsModel;

    bool isMovie(const QModelIndex& sourceIndex) const;
    bool isMovieDiscDirectory(const QModelIndex& sourceIndex) const;
    bool anyChildrenIsMovie(const QModelIndex& sourceIndex) const;

    void notifyRowUpdate(QString path);
    void notifyRowUpdate(QString path, int startColumn, int endColumn);

private slots:
    void onMediaDbKeyChangedForPath(QString, QString);
};

#endif // MEDIAMODEL_H
