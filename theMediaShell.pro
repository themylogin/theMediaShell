#-------------------------------------------------
#
# Project created by QtCreator 2013-01-26T17:47:17
#
#-------------------------------------------------

QT       += core gui network widgets x11extras

TARGET = theMediaShell
TEMPLATE = app

CONFIG += link_pkgconfig debug
PKGCONFIG += libavformat libavcodec libavutil x11 mpv libmpdclient alsa

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

QMAKE_CXXFLAGS += -std=c++11

RESOURCES += \
    main.qrc

OTHER_FILES += \
    MainWindow.qss \
    Player/PlayerWindowSidebar.qss \
    QMessageBox.qss
