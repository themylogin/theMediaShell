#-------------------------------------------------
#
# Project created by QtCreator 2013-01-26T17:47:17
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = theMediaShell
TEMPLATE = app

LIBS += -lX11 -lqjson -lmpv -lmpdclient

SOURCES += MediaModel/MediaModel.cpp \
    Application.cpp \
    main.cpp \
    Utils.cpp \
    Player/PlayerWindow.cpp \
    MainWindow/MpdDialog.cpp \
    MainWindow/MainWindow.cpp \
    MainWindow/MpdClient.cpp

HEADERS  += MediaModel/Helper/QFileSystemModelWithMappedColumns.h \
    MediaModel/Helper/QFileSystemProxyModelMixin.h \
    MediaModel/MediaModel.h \
    Player/PlaylistModel.h \
    Player/PlaylistItem.h \
    Player/PlayerWindow.h \
    Application.h \
    Utils.h \
    MediaDb.h \
    MainWindow/MpdDialog.h \
    MainWindow/MainWindow.h \
    MainWindow/MpdClient.h

FORMS    +=

QMAKE_CXXFLAGS += -std=c++0x

CONFIG  += qxt
QXT     += core gui

RESOURCES += \
    main.qrc

OTHER_FILES += \
    MainWindow.qss \
    Player/PlayerWindow.qss \
    QMessageBox.qss
