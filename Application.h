#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#include <QX11Info>

class Application : public QApplication
{
    Q_OBJECT

public:
    Application(int& argc, char** argv);
    bool x11EventFilter(XEvent* event);

private:
    Display* dpy;
    int xkb_event_type;
};

#endif // APPLICATION_H
