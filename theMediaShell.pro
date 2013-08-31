#-------------------------------------------------
#
# Project created by QtCreator 2013-01-26T17:47:17
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = theMediaShell
TEMPLATE = app

LIBS += -lX11 -lqjson -lSimpleAmqpClient

SOURCES += main.cpp \
    Application.cpp \
    AmqpConnectionThread.cpp \
    Utils.cpp \
    VideoIdentifier.cpp \
    MediaModel/MediaModel.cpp \
    Classificators.cpp \
    RenameAbandonedSubtitlesDialog.cpp

HEADERS  += MainWindow.h \
    MediaModel/MediaModel.h \
    MediaModel/NewMediaModel.h \
    MediaModel/Helper/FlatMediaModel.h \
    MediaModel/Helper/QFileSystemModelWithMappedColumns.h \
    MediaModel/Helper/QFileSystemProxyModelMixin.h \
    MediaClassificator.h \
    ExtensionMediaClassificator.h \
    MplayerWindow.h \
    PlaylistItem.h \
    PlaylistModel.h \
    Application.h \
    MediaConsumptionHistory.h \
    AmqpConnectionThread.h \
    Utils.h \
    VideoIdentifier.h \
    VideoIdentification.h \
    Classificators.h \
    RenameAbandonedSubtitlesDialog.h

FORMS    +=

QMAKE_CXXFLAGS += -std=c++0x

CONFIG  += qxt
QXT     += core gui
