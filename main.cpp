#include <iostream>

#include <QThreadPool>
#include <QVector>

#include "Application.h"
#include "MainWindow/MainWindow.h"

int main(int argc, char* argv[])
{
    QThreadPool::globalInstance()->setMaxThreadCount(1);

    QCoreApplication::setOrganizationName("thelogin.ru");
    QCoreApplication::setOrganizationDomain("thelogin.ru");
    QCoreApplication::setApplicationName("theMediaShell");

    qRegisterMetaType<QVector<int>>("QVector<int>");

    Application a(argc, argv);
    MainWindow mainWindow(argv[1]);
    mainWindow.showFullScreen();    
    return a.exec();
}
