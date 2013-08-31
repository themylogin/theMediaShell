#ifndef EXTENSIONMEDIACLASSIFICATOR_H
#define EXTENSIONMEDIACLASSIFICATOR_H

#include <QList>

#include "MediaClassificator.h"

class ExtensionMediaClassificator : public MediaClassificator
{
public:
    ExtensionMediaClassificator& addExtension(QString extension)
    {
        this->pathEndings.append("." + extension.toLower());
        return *this;
    }

    virtual bool is(QString path)
    {
        foreach (QString pathEnding, this->pathEndings)
        {
            if (path.endsWith(pathEnding))
            {
                return true;
            }
        }
        return false;
    }

private:
    QList<QString> pathEndings;
};

#endif // EXTENSIONMEDIACLASSIFICATOR_H
