#include "Application.h"
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("thelogin.ru");
    QCoreApplication::setOrganizationDomain("thelogin.ru");
    QCoreApplication::setApplicationName("theMediaShell");

    Application a(argc, argv);
    MainWindow w;
    w.showFullScreen();
    
    return a.exec();
}
