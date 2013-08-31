#include <iostream>

#include <QThreadPool>

#include "Application.h"
#include "Classificator/Classificators.h"
#include "Classificator/ExtensionMediaClassificator.h"
#include "MediaHandler/MovieHandler.h"
#include "MediaHandler/ShellCommandHandler.h"
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
    MainWindow mainWindow;

    //
    MovieHandler* movieHandler = new MovieHandler;

    MediaModel* moviesModel = new MediaModel("/home/themylogin/Storage/Torrent/downloads", extensionVideoClassificator);
    mainWindow.addTree(QString::fromUtf8("Фильмы"), moviesModel, moviesModel->rootIndex(), movieHandler);

    NewMediaModel* newMoviesModel = new NewMediaModel(moviesModel);
    mainWindow.addTable(QString::fromUtf8("Новинки"), newMoviesModel, movieHandler);

    //
    auto nesClassificator = new ExtensionMediaClassificator;
    nesClassificator->addExtension("nes");
    MediaModel* nesModel = new MediaModel("/home/themylogin/ROMs/NES ROMs", nesClassificator);
    mainWindow.addTree(QString::fromUtf8("NES"), nesModel, nesModel->rootIndex(), new ShellCommandHandler("mednafen %1"));

    mainWindow.showFullScreen();
    
    return a.exec();
}
