#-------------------------------------------------
#
# Project created by QtCreator 2013-01-26T17:47:17
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = theMediaShell
TEMPLATE = app

LIBS += -lX11 -lqjson

SOURCES += main.cpp \
    Application.cpp \
    Utils.cpp \
    VideoIdentifier.cpp \
    RenameAbandonedSubtitlesDialog.cpp \
    Classificator/Classificators.cpp \
    MediaModel/MediaModel.cpp \
    MediaHandler/MovieHandler.cpp \
    MediaHandler/ShellCommandHandler.cpp

HEADERS  += MainWindow.h \
    Classificator/Classificators.h \
    Classificator/MediaClassificator.h \
    Classificator/ExtensionMediaClassificator.h \
    MediaModel/MediaModel.h \
    MediaModel/NewMediaModel.h \
    MediaModel/Helper/FlatMediaModel.h \
    MediaModel/Helper/QFileSystemModelWithMappedColumns.h \
    MediaModel/Helper/QFileSystemProxyModelMixin.h \
    MplayerWindow.h \
    PlaylistItem.h \
    PlaylistModel.h \
    Application.h \
    MediaConsumptionHistory.h \
    Utils.h \
    VideoIdentifier.h \
    VideoIdentification.h \
    RenameAbandonedSubtitlesDialog.h \
    MediaHandler/MediaHandler.h \
    MediaHandler/MovieHandler.h \
    MediaHandler/ShellCommandHandler.h

FORMS    +=

QMAKE_CXXFLAGS += -std=c++0x

CONFIG  += qxt
QXT     += core gui

RESOURCES += \
    main.qrc

OTHER_FILES += \
    MainWindow.qss
