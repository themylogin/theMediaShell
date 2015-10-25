#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QEvent>
#include <QLabel>
#include <QMainWindow>
#include <QTimer>
#include <QTreeView>

#include "MainWindow/PlayerSessionCountdown.h"
#include "MediaModel/MediaModel.h"
#include "Mpd/MpdClient.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    MainWindow(QString mediaPath, QWidget* parent = 0);
    
protected:
    bool eventFilter(QObject* object, QEvent* event);

private:
    MediaModel* model;
    QTreeView* view;

    MpdClient* mpd;
    bool mpdWasPlaying;

    bool focusFirstItemConnected;

    bool inPlayerSession;
    QLabel* statusBarLabel;
    PlayerSessionCountdown* playerSessionCountdown;

private slots:
    void focusFirstItem();
    void scrollToCurrentItem();
    void mediaActivated(QModelIndex media);
    void playerWindowClosing(bool powerOff);
    void playerSessionMessage(QString message);
    void playerSessionFinished();
};

#endif // MAINWINDOW_H
