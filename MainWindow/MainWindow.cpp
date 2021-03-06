#include "MainWindow.h"

#include <QFile>
#include <QKeyEvent>
#include <QMessageBox>
#include <QProcess>
#include <QStatusBar>
#include <QStringList>

#include "Hook.h"
#include "MediaDb.h"
#include "Mpd/MpdDialog.h"
#include "Player/PlayerWindow.h"
#include "Utils.h"

MainWindow::MainWindow(QString mediaPath, QWidget* parent)
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

    QThread* mpdThread = new QThread;
    this->mpd = new MpdClient;
    mpd->moveToThread(mpdThread);
    connect(mpdThread, SIGNAL(started()), mpd, SLOT(run()));
    mpdThread->start();

    this->inPlayerSession = false;
    this->playerSessionCountdown = new PlayerSessionCountdown(5, this);
    connect(this->playerSessionCountdown, SIGNAL(message(QString)), this, SLOT(playerSessionMessage(QString)));
    connect(this->playerSessionCountdown, SIGNAL(timedOut()), this, SLOT(playerSessionFinished()));

    this->statusBarLabel = new QLabel(this);
    this->statusBarLabel->setAlignment(Qt::AlignCenter);
    this->statusBar()->addWidget(this->statusBarLabel, 1);
    this->statusBar()->hide();
}

bool MainWindow::eventFilter(QObject* object, QEvent* event)
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

            this->playerSessionCountdown->stop();
            this->statusBar()->hide();

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

            if (keyEvent->key() == Qt::Key_F5)
            {
                this->model->forceUpdate();
            }

            if (keyEvent->key() == Qt::Key_Space)
            {
                auto index = this->view->currentIndex();
                if (!this->model->isDir(index))
                {
                    index = index.parent();
                }
                if (index.isValid())
                {
                    QProcess::startDetached("urxvt", QStringList() << "-e" << "mc" << this->model->filePath(index));
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
                Utils::resizeMessageBox(&messageBox);
                if (messageBox.exec() == QMessageBox::Yes)
                {
                    this->model->remove(index);
                }
            }

            if (keyEvent->key() == Qt::Key_Backspace)
            {
                auto index = this->view->currentIndex();
                if (!this->model->isDir(index))
                {
                    index = index.parent();
                }
                if (index.isValid())
                {
                    auto playlistTitle = this->model->filePath(index);
                    MediaDb::getInstance().del(playlistTitle, "openingLength");
                    MediaDb::getInstance().del(playlistTitle, "endingLength");
                }
            }
        }
    }

    return false;
}

void MainWindow::focusFirstItem()
{
    this->view->setCurrentIndex(this->model->index(0, 0, this->view->rootIndex()));
}

void MainWindow::scrollToCurrentItem()
{
    this->view->scrollTo(this->view->currentIndex());
}

void MainWindow::mediaActivated(QModelIndex media)
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

        if (MpdDialog::waitMusicOver(playlistTitle, this->mpd, true, &this->mpdWasPlaying, this))
        {
            if (!this->inPlayerSession)
            {
                this->inPlayerSession = true;
                Hook::run("pre-player-session");
            }

            PlayerWindow* playerWindow = new PlayerWindow(playlistName, playlistTitle, playlist,
                                                          this->mpd, &this->mpdWasPlaying);
            connect(playerWindow, SIGNAL(closing(bool)), this, SLOT(playerWindowClosing(bool)));
        }
    }
}

void MainWindow::playerWindowClosing(bool powerOff)
{
    if (powerOff)
    {
        this->inPlayerSession = false;
        Hook::run("post-player-session");

        Hook::run("power-off");
    }
    else
    {
        this->playerSessionCountdown->start();
    }
}

void MainWindow::playerSessionMessage(QString message)
{
    this->statusBarLabel->setText(message);
    this->statusBar()->show();
}

void MainWindow::playerSessionFinished()
{
    this->statusBar()->hide();

    this->inPlayerSession = false;
    Hook::run("post-player-session");

    if (this->mpdWasPlaying)
    {
        this->mpd->resume();
    }
}
