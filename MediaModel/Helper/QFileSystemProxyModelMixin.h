#ifndef QFILESYSTEMPROXYMODELMIXIN_H
#define QFILESYSTEMPROXYMODELMIXIN_H

#include <QAbstractProxyModel>
#include <QDateTime>
#include <QFileSystemModel>

class QFileSystemProxyModelMixin
{
public:
    QFileSystemProxyModelMixin(QAbstractProxyModel* child)
    {
        this->child = child;

        this->sourceFileSystemModel = NULL;
    }

    #define PROXY(type, getter)\
        type getter(const QModelIndex& index) const\
        {\
            return this->sourceFileSystemModel->getter(this->child->mapToSource(index));\
        }

    PROXY(bool, isDir)
    PROXY(QString, fileName)
    PROXY(QString, filePath)
    PROXY(QDateTime, lastModified)

    PROXY(bool, remove)

protected:
    void setSourceFileSystemModel(QFileSystemModel* sourceFileSystemModel)
    {
        this->sourceFileSystemModel = sourceFileSystemModel;
    }

private:
    QAbstractProxyModel* child;

    QFileSystemModel* sourceFileSystemModel;
};

#endif // QFILESYSTEMPROXYMODELMIXIN_H
