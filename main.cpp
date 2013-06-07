#include <iostream>

#include "Application.h"
#include "MainWindow.h"

int main(int argc, char* argv[])
{
    QCoreApplication::setOrganizationName("thelogin.ru");
    QCoreApplication::setOrganizationDomain("thelogin.ru");
    QCoreApplication::setApplicationName("theMediaShell");

    Application a(argc, argv);
    
    if (a.arguments().count() != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <movies directory>" << std::endl;
        return 1;
    }
    
    MainWindow w(a.arguments()[1]);
    w.showFullScreen();
    
    return a.exec();
}
