#include <QDebug>
#include <QX11Info>

#include "Application.h"

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
}

bool Application::x11EventFilter(XEvent* event)
{
    if (event->type == this->xkb_event_type)
    {
        XkbEvent* xkbev = (XkbEvent*)event;
        if (xkbev->any.xkb_type == XkbStateNotify)
        {
            XkbLockGroup(this->dpy, XkbUseCoreKbd, 0); // do not allow layout any other from english
        }
    }

    return false;
}
