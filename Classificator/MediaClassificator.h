#ifndef MEDIACLASSIFICATOR_H
#define MEDIACLASSIFICATOR_H

#include <QString>

class MediaClassificator
{
public:
    virtual bool is(QString path) = 0;
};

#endif // MEDIACLASSIFICATOR_H
