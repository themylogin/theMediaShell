#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QEvent>
#include <QFile>
#include <QKeyEvent>
#include <QMainWindow>
#include <QMessageBox>
#include <QStringList>
#include <QTreeView>

#include "MediaModel/MediaModel.h"
#include "Player/PlayerWindow.h"
#include "Utils.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    MainWindow(QString mediaPath, QWidget* parent = 0)
        : QMainWindow(parent)
    {
        Utils::setStyleSheetFromFile(this, "://MainWindow.qss");

        this->model = new MediaModel(mediaPath);

        this->view = new QTreeView(this);
        this->view->setModel(model);
        this->view->setRootIndex(this->model->rootIndex());
        this->view->setHeaderHidden(true);
        this->view->setColumnWidth(0, 1205);  // Name
        this->view->setColumnWidth(1, 240);   // Time
        this->view->setColumnWidth(2, 175);   // Size
        this->view->setColumnWidth(3, 270);   // Date Modified
        this->view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        this->view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        this->view->setIconSize(QSize(32, 32));
        this->focusFirstItemConnected = true;
        connect(this->model, SIGNAL(layoutChanged()), this, SLOT(focusFirstItem()));
        connect(this->model, SIGNAL(layoutChanged()), this, SLOT(scrollToCurrentItem()));
        connect(this->view, SIGNAL(activated(QModelIndex)), this, SLOT(mediaActivated(QModelIndex)));
        this->view->installEventFilter(this);
        this->setCentralWidget(this->view);
    }
    
protected:
    bool eventFilter(QObject* object, QEvent* event)
    {
        if (object == this->view)
        {
            if (event->type() == QEvent::KeyPress)
            {
                if (this->focusFirstItemConnected)
                {
                    this->focusFirstItemConnected = false;
                    disconnect(this->model, SIGNAL(layoutChanged()), this, SLOT(focusFirstItem()));
                }

                QKeyEvent* keyEvent = dynamic_cast<QKeyEvent*>(event);

                if (keyEvent->key() == Qt::Key_Down)
                {
                    int y = this->view->visualRect(this->view->currentIndex()).top() + 49 * 5;
                    if (y > this->view->viewport()->rect().top())
                    {
                        QModelIndex itemProvidesConfidence = this->view->indexAt(QPoint(this->view->viewport()->rect().left(), y));
                        if (itemProvidesConfidence.isValid())
                        {
                            this->view->scrollTo(itemProvidesConfidence);
                        }
                    }
                }
                if (keyEvent->key() == Qt::Key_Up)
                {
                    int y = this->view->visualRect(this->view->currentIndex()).top() - 49 * 5;
                    if (y < this->view->viewport()->rect().top())
                    {
                        QModelIndex itemProvidesConfidence = this->view->indexAt(QPoint(this->view->viewport()->rect().left(), y));
                        if (itemProvidesConfidence.isValid())
                        {
                            this->view->scrollTo(itemProvidesConfidence);
                        }
                    }
                }

                if (keyEvent->key() == Qt::Key_Left)
                {
                    if (!this->view->isExpanded(this->view->currentIndex()))
                    {
                        auto parentIndex = this->view->currentIndex().parent();
                        if (parentIndex.isValid() && parentIndex != this->view->rootIndex())
                        {
                            this->view->collapse(parentIndex);
                            this->view->setCurrentIndex(parentIndex);
                        }
                    }
                }

                if (keyEvent->key() == Qt::Key_Delete)
                {
                    auto index = this->view->currentIndex();

                    QMessageBox messageBox(QMessageBox::Question,
                                           "Are you sure?",
                                           QString("Are you sure want to delete %1?").arg(this->model->fileName(index)),
                                           QMessageBox::Yes | QMessageBox::No,
                                           this,
                                           Qt::Dialog | Qt::FramelessWindowHint);
                    messageBox.setDefaultButton(QMessageBox::Yes);
                    Utils::setStyleSheetFromFile(&messageBox, "://QMessageBox.qss");
                    if (messageBox.exec() == QMessageBox::Yes)
                    {
                        this->model->remove(index);
                    }
                }
            }
        }

        return false;
    }

private:
    MediaModel* model;
    QTreeView* view;

    bool focusFirstItemConnected;

private slots:
    void focusFirstItem()
    {
        this->view->setCurrentIndex(this->model->index(0, 0, this->view->rootIndex()));
    }

    void scrollToCurrentItem()
    {
        this->view->scrollTo(this->view->currentIndex());
    }

    void mediaActivated(QModelIndex media)
    {
        if (this->model->hasChildren(media))
        {
            if (this->view->isExpanded(media))
            {
                this->view->collapse(media);
            }
            else
            {
                this->view->expand(media);
            }
        }
        else
        {
            QString playlistName;
            QString playlistTitle;
            QStringList playlist;
            if (media.parent() == this->model->rootIndex())
            {
                playlistName = "";

                playlistTitle = this->model->fileName(media);

                playlist.append(this->model->filePath(media));
            }
            else
            {                
                playlistName = this->model->filePath(media.parent());

                playlistTitle = this->model->fileName(media.parent());

                auto item = media;
                while (item.isValid())
                {
                    playlist.append(this->model->filePath(item));
                    item = item.sibling(item.row() + 1, 0);
                }
            }

            PlayerWindow* playerWindow = new PlayerWindow(playlistName, playlistTitle, playlist);
            Q_UNUSED(playerWindow);
        }
    }
};

#endif // MAINWINDOW_H
