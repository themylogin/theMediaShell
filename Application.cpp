#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QX11Info>

#include "Application.h"

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>

Application::Application(int& argc, char** argv)
    : QApplication(argc, argv)
{
    this->dpy = QX11Info::display();

    Atom a_XKB_RULES_NAMES = XInternAtom(this->dpy, "_XKB_RULES_NAMES", False);
    if (a_XKB_RULES_NAMES == None)
    {
        qDebug() << "XInternAtom(this->dpy, \"_XKB_RULES_NAMES\", False) == None";
        return;
    }

    int dummy;
    if (!XkbQueryExtension(this->dpy, &dummy, &this->xkb_event_type, &dummy, &dummy, &dummy))
    {
        qDebug() << "XkbQueryExtension(this->dpy, &dummy, &this->xkb_event_type, &dummy, &dummy, &dummy) == 0";
        return;
    }
    XkbSelectEventDetails(this->dpy, XkbUseCoreKbd, XkbStateNotify, XkbAllStateComponentsMask, XkbGroupStateMask);

    XkbStateRec xkb_state;
    if (XkbGetState(this->dpy, XkbUseCoreKbd, &xkb_state) != Success)
    {
        qDebug() << "XkbGetState(this->dpy, XkbUseCoreKbd, &xkb_state) != Success";
        return;
    }

    this->_NET_ACTIVE_WINDOW = XInternAtom(this->dpy, "_NET_ACTIVE_WINDOW", False);
    this->_NET_WM_PID = XInternAtom(this->dpy, "_NET_WM_PID", False);

    this->setEnglishLayout();
}

bool Application::x11EventFilter(XEvent* event)
{
    if (event->type == this->xkb_event_type)
    {
        XkbEvent* xkbev = (XkbEvent*)event;
        if (xkbev->any.xkb_type == XkbStateNotify)
        {
            auto activeWindow = this->getActiveWindow();
            if (activeWindow)
            {
                auto pid = this->getWindowPID(activeWindow);
                if (pid)
                {
                    auto processName = this->getProcessName(pid);
                    if (processName == "mpv" ||
                        processName == "theMediaShell")
                    {
                        this->setEnglishLayout(); // do not allow layout any other from english
                    }
                }
            }
        }
    }

    return false;
}

Window Application::getActiveWindow()
{
    Atom actual_type;
    int actual_format;
    unsigned long nitems = 0;
    unsigned long bytes_after;
    unsigned char* current_window_prop;
    XGetWindowProperty(this->dpy, XRootWindow(this->dpy, 0), this->_NET_ACTIVE_WINDOW, 0, (~0L),
                       False, AnyPropertyType, &actual_type,
                       &actual_format, &nitems, &bytes_after,
                       &current_window_prop);
    if (nitems > 0 && *current_window_prop > 0)
    {
        return *((Window*)current_window_prop);
    }
    return 0;
}

unsigned long Application::getWindowPID(Window window)
{
    Atom actual_type;
    int actual_format;
    unsigned long nitems = 0;
    unsigned long bytes_after;
    unsigned char* pid_prop;
    if (Success == XGetWindowProperty(this->dpy, window, this->_NET_WM_PID, 0, 1,
                                      False, XA_CARDINAL, &actual_type,
                                      &actual_format, &nitems, &bytes_after,
                                      &pid_prop))
    {
        if (pid_prop != 0)
        {
            return *((unsigned long*)pid_prop);
        }
    }
    return 0;
}

QString Application::getProcessName(unsigned long pid)
{
    QFile file(QString("/proc/%1/cmdline").arg(pid));
    if (file.open(QIODevice::ReadOnly))
    {
        return QFileInfo(QString::fromUtf8(file.readAll().constData())).fileName();
    }
    return QString();
}

void Application::setEnglishLayout()
{
    XkbLockGroup(this->dpy, XkbUseCoreKbd, 0);
}
