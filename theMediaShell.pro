#-------------------------------------------------
#
# Project created by QtCreator 2013-01-26T17:47:17
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = theMediaShell
TEMPLATE = app

LIBS += -lX11 -lqjson -lmpv

SOURCES += MediaModel/MediaModel.cpp \
    Application.cpp \
    main.cpp \
    Utils.cpp

HEADERS  += MediaModel/Helper/QFileSystemModelWithMappedColumns.h \
    MediaModel/Helper/QFileSystemProxyModelMixin.h \
    MediaModel/MediaModel.h \
    Player/PlaylistModel.h \
    Player/PlaylistItem.h \
    Player/PlayerWindow.h \
    Application.h \
    MainWindow.h \
    Utils.h \
    MediaDb.h

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
