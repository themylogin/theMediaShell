#include <iostream>

#include <QThreadPool>

#include "Application.h"
#include "Classificator/Classificators.h"
#include "Classificator/ExtensionMediaClassificator.h"
#include "MediaHandler/MovieHandler.h"
#include "MediaModel/MediaModel.h"
#include "MediaModel/NewMediaModel.h"
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
    
    MainWindow mainWindow;

    MovieHandler* movieHandler = new MovieHandler;

    MediaModel* moviesModel = new MediaModel(a.arguments()[1], extensionVideoClassificator);
    mainWindow.addTree(QString::fromUtf8("Фильмы"), moviesModel, moviesModel->rootIndex(), movieHandler);

    NewMediaModel* newMoviesModel = new NewMediaModel(moviesModel);
    mainWindow.addTable(QString::fromUtf8("Новинки"), newMoviesModel, movieHandler);

    mainWindow.showFullScreen();
    
    return a.exec();
}
