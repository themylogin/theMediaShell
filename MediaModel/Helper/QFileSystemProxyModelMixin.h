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
        this->sourceFileSystemProxyModel = NULL;
    }

    #define GETTER(type, getter)\
        type getter(const QModelIndex& index) const\
        {\
            if (this->sourceFileSystemModel)\
            {\
                return this->sourceFileSystemModel->getter(this->child->mapToSource(index));\
            }\
            else\
            {\
                return this->sourceFileSystemProxyModel->getter(this->child->mapToSource(index));\
            }\
        }

    GETTER(bool, isDir)
    GETTER(QString, fileName)
    GETTER(QString, filePath)
    GETTER(QDateTime, lastModified)

protected:
    void setSourceFileSystemModel(QFileSystemModel* sourceFileSystemModel)
    {
        this->sourceFileSystemModel = sourceFileSystemModel;
    }

    void setSourceFileSystemProxyModel(QFileSystemProxyModelMixin* sourceFileSystemProxyModel)
    {
        this->sourceFileSystemProxyModel = sourceFileSystemProxyModel;
    }

private:
    QAbstractProxyModel* child;

    QFileSystemModel* sourceFileSystemModel;
    QFileSystemProxyModelMixin* sourceFileSystemProxyModel;
};

#endif // QFILESYSTEMPROXYMODELMIXIN_H
