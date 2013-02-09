#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QEvent>
#include <QFileSystemModel>
#include <QHeaderView>
#include <QKeyEvent>
#include <QMainWindow>
#include <QStringList>
#include <QTableView>
#include <QTabWidget>
#include <QTimer>
#include <QTreeView>

#include "ExtensionMediaClassificator.h"
#include "NewMediaModel.h"
#include "MediaModel.h"
#include "MplayerWindow.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    MainWindow(QWidget* parent = 0)
        : QMainWindow(parent)
    {
        this->setStyleSheet(R"(
            *
            {
                background-color: rgb(17, 9, 15);
            }

            QTabWidget::pane
            {
                border: none;
            }

            QTabBar
            {
                background-color: rgb(17, 9, 15);
                color: rgb(166, 164, 166);
                font: 60px "Segoe UI";
            }
            QTabBar::tab
            {
                border: none;
                padding: 12px 0 0 25px;
                font: 48px "Segoe UI";
            }
            QTabBar::tab:selected
            {
                color: rgb(240, 241, 240);
                padding: 0px 0 0 25px;
                font: 60px "Segoe UI";
            }

            /***/
            QTreeView, QTableView
            {
                border: none;
                outline: none;
                font: 35px "Segoe UI";
                selection-background-color: #fff;
            }
            QTableView
            {
                margin-left: 20px;
            }
            QTreeView::item:selected, QTableView::item:selected
            {
                background-color: rgb(17, 9, 15);
                color: rgb(240, 241, 240);
            }

            QTreeView::branch {
                background: palette(base);
            }
            QTreeView::branch:has-siblings:!adjoins-item {
                background-color: rgb(17, 9, 15);
            }
            QTreeView::branch:has-siblings:adjoins-item {
                background-color: rgb(17, 9, 15);
            }
            QTreeView::branch:!has-children:!has-siblings:adjoins-item {
                background-color: rgb(17, 9, 15);
            }
            QTreeView::branch:closed:has-children:has-siblings {
                background-color: rgb(17, 9, 15);
            }
            QTreeView::branch:has-children:!has-siblings:closed {
                background-color: rgb(17, 9, 15);
            }
            QTreeView::branch:open:has-children:has-siblings {
                background-color: rgb(17, 9, 15);
            }
            QTreeView::branch:open:has-children:!has-siblings {
                background-color: rgb(17, 9, 15);
            }
        )");

        this->tabWidget = new QTabWidget(this);
        this->tabWidget->setFocusPolicy(Qt::NoFocus);
        this->setCentralWidget(tabWidget);

        this->moviesClassificator = new ExtensionMediaClassificator;
        this->moviesClassificator->addExtension("avi");
        this->moviesClassificator->addExtension("mkv");
        this->moviesClassificator->addExtension("mp4");
        this->moviesModel = new MediaModel("/home/themylogin/Torrent/downloads", this->moviesClassificator, this);
        this->moviesView = new QTreeView(this->tabWidget);
        this->moviesView->setModel(this->moviesModel);
        this->moviesView->setHeaderHidden(true);
        this->suitUpView(this->moviesView);
        connect(this->moviesView, SIGNAL(activated(QModelIndex)), this, SLOT(movieActivated(QModelIndex)));
        this->tabWidget->addTab(this->moviesView, QString::fromUtf8("Фильмы"));

        // Delaying this somehow prevents startup crashes in QSortFilterProxyModel::parent()
        this->setMoviesViewRootIndexTimer = new QTimer(this);
        connect(this->setMoviesViewRootIndexTimer, SIGNAL(timeout()), this, SLOT(setMoviesViewRootIndex()));
        this->setMoviesViewRootIndexTimer->start();

        this->newMoviesModel = new NewMediaModel(this->moviesModel, this);
        this->newMoviesView = new QTableView(this->tabWidget);
        this->newMoviesView->setModel(this->newMoviesModel);
        this->newMoviesView->setSortingEnabled(true);
        this->newMoviesView->horizontalHeader()->hide();
        this->newMoviesView->verticalHeader()->hide();
        this->newMoviesView->verticalHeader()->setDefaultSectionSize(49); // TODO: Find a way to put this into stylesheet
        this->newMoviesView->setSelectionBehavior(QAbstractItemView::SelectRows);
        this->newMoviesView->setShowGrid(false);
        this->suitUpView(this->newMoviesView);
        this->newMoviesView->setColumnWidth(0, this->newMoviesView->columnWidth(0) - 20); // TODO: because of margin in stylesheet, this is awful
        this->tabWidget->addTab(this->newMoviesView, QString::fromUtf8("Новинки"));
    }

    ~MainWindow()
    {

    }
    
protected:
    template<typename T>
    void suitUpView(T* view)
    {
        view->setColumnWidth(0, 1430);  // Name
        view->setColumnWidth(1, 200);   // Size
        view->setColumnHidden(2, true); // Type
        view->setColumnWidth(3, 250);   // Date Modified

        view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        view->setIconSize(QSize(32, 32));

        view->installEventFilter(this);
    }

    bool eventFilter(QObject* object, QEvent* event)
    {
        Q_UNUSED(object);

        if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent* keyEvent = dynamic_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Tab)
            {
                this->tabWidget->setCurrentIndex((this->tabWidget->currentIndex() + 1) % this->tabWidget->count());
                return true;
            }
        }

        return false;
    }

private:
    QTabWidget* tabWidget;

    ExtensionMediaClassificator* moviesClassificator;
    MediaModel* moviesModel;
    QTreeView* moviesView;

    NewMediaModel* newMoviesModel;
    QTableView* newMoviesView;

    QTimer* setMoviesViewRootIndexTimer;

private slots:
    void setMoviesViewRootIndex()
    {
        this->moviesView->setRootIndex(this->moviesModel->rootIndex());
    }

    void movieActivated(QModelIndex movie)
    {
        QString title;
        QStringList playlist;
        if (movie.parent() == this->moviesModel->rootIndex())
        {
            title = movie.data().toString();
            playlist.append(this->moviesModel->filePath(movie));
        }
        else
        {
            title = movie.parent().data().toString();
            while (movie.isValid())
            {
                playlist.append(this->moviesModel->filePath(movie));
                movie = movie.sibling(movie.row() + 1, 0);
            }
        }

        MplayerWindow* mplayer = new MplayerWindow(title, playlist);
        mplayer->show();
    }
};

#endif // MAINWINDOW_H
