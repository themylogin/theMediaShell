#include <iostream>

#include <QThreadPool>

#include "Application.h"
#include "Classificators.h"
#include "ExtensionMediaClassificator.h"
#include "MainWindow.h"

int main(int argc, char* argv[])
{
    QThreadPool::globalInstance()->setMaxThreadCount(1);

    QCoreApplication::setOrganizationName("thelogin.ru");
    QCoreApplication::setOrganizationDomain("thelogin.ru");
    QCoreApplication::setApplicationName("theMediaShell");

    auto extensionVideoClassificator = new ExtensionMediaClassificator;
    extensionVideoClassificator->addExtension("avi");
    extensionVideoClassificator->addExtension("m4v");
    extensionVideoClassificator->addExtension("mkv");
    extensionVideoClassificator->addExtension("mov");
    extensionVideoClassificator->addExtension("mp4");
    extensionVideoClassificator->addExtension("webm");
    videoClassificator = extensionVideoClassificator;

    auto extensionSubtitleClassificator = new ExtensionMediaClassificator;
    extensionSubtitleClassificator->addExtension("ass");
    extensionSubtitleClassificator->addExtension("srt");
    extensionSubtitleClassificator->addExtension("sub");
    subtitleClassificator = extensionSubtitleClassificator;

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
