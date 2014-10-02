#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <mpd/client.h>

#include <QEvent>
#include <QMainWindow>
#include <QTreeView>

#include "MediaModel/MediaModel.h"

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

    bool focusFirstItemConnected;

private slots:
    void focusFirstItem();
    void scrollToCurrentItem();
    void mediaActivated(QModelIndex media);
};

#endif // MAINWINDOW_H
