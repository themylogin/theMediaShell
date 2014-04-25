#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#include <QString>
#include <QX11Info>

#include <X11/Xdefs.h>
typedef XID Window;

class Application : public QApplication
{
    Q_OBJECT

public:
    Application(int& argc, char** argv);
    bool x11EventFilter(XEvent* event);

private:
    Display* dpy;
    int xkb_event_type;
    Atom _NET_ACTIVE_WINDOW;
    Atom _NET_WM_PID;

    Window getActiveWindow();
    unsigned long getWindowPID(Window window);
    QString getProcessName(unsigned long pid);
    void setEnglishLayout();
};

#endif // APPLICATION_H
