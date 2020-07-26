#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#include <QString>

class Application : public QApplication
{
    Q_OBJECT

public:
    Application(int& argc, char** argv);

private:
    void setEnglishLayout();
};

#endif // APPLICATION_H
