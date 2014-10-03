#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QEvent>
#include <QMainWindow>
#include <QTreeView>

#include "MediaModel/MediaModel.h"
#include "MpdClient.h"

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

private slots:
    void focusFirstItem();
    void scrollToCurrentItem();
    void mediaActivated(QModelIndex media);
    void playerWindowClosing(bool poweringOff);
};

#endif // MAINWINDOW_H
