#-------------------------------------------------
#
# Project created by QtCreator 2013-01-26T17:47:17
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = theMediaShell
TEMPLATE = app


SOURCES += main.cpp

HEADERS  += MainWindow.h \
    MediaModel.h \
    MediaClassificator.h \
    ExtensionMediaClassificator.h \
    NewMediaModel.h \
    FlatMediaModel.h \
    MplayerWindow.h \
    PlaylistItem.h \
    PlaylistModel.h

FORMS    +=

QMAKE_CXXFLAGS += -std=c++0x

CONFIG  += qxt
QXT     += core gui