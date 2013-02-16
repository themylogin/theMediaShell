#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QEvent>
#include <QFileSystemModel>
#include <QHeaderView>
#include <QKeyEvent>
#include <QMainWindow>
#include <QSignalMapper>
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
            QTreeView::item, QTableView::item
            {
                padding-right: 20px;
            }
            QTreeView::item:selected, QTableView::item:selected
            {
                background-color: rgb(17, 9, 15);
                color: rgb(240, 241, 240);
            }
            QTreeView::indicator:checked, QTableView::indicator:checked
            {
                image: none;
                color: rgb(120, 120, 120);
            }
            QTreeView::item:checked, QTableView::item:checked
            {
                color: rgb(120, 120, 120);
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

        this->focusFirstItemMapper = new QSignalMapper(this);
        connect(this->focusFirstItemMapper, SIGNAL(mapped(QObject*)), this, SLOT(focusFirstItem(QObject*)));
        this->scrollToCurrentItemMapper = new QSignalMapper(this);
        connect(this->scrollToCurrentItemMapper, SIGNAL(mapped(QObject*)), this, SLOT(scrollToCurrentItem(QObject*)));

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
        QTimer::singleShot(0, this, SLOT(setMoviesViewRootIndex()));

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
        view->setColumnWidth(3, 270);   // Date Modified

        view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        view->setIconSize(QSize(32, 32));

        view->installEventFilter(this);

        this->focusFirstItemMapper->setMapping((QObject*)view->model(), (QObject*)view);
        connect(view->model(), SIGNAL(layoutChanged()), this->focusFirstItemMapper, SLOT(map()));
        this->scrollToCurrentItemMapper->setMapping((QObject*)view->model(), (QObject*)view);
        connect(view->model(), SIGNAL(layoutChanged()), this->scrollToCurrentItemMapper, SLOT(map()));
    }

    bool eventFilter(QObject* object, QEvent* event)
    {
        if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent* keyEvent = dynamic_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Tab)
            {
                this->tabWidget->setCurrentIndex((this->tabWidget->currentIndex() + 1) % this->tabWidget->count());
                return true;
            }
        }

        QAbstractItemView* view = qobject_cast<QAbstractItemView*>(object);
        if (view)
        {
            if (event->type() == QEvent::KeyPress)
            {
                this->focusFirstItemMapper->removeMappings(view->model());

                QKeyEvent* keyEvent = dynamic_cast<QKeyEvent*>(event);
                if (keyEvent->key() == Qt::Key_Down)
                {
                    int y = view->visualRect(view->currentIndex()).top() + 49 * 5;
                    if (y > view->viewport()->rect().top())
                    {
                        QModelIndex itemProvidesConfidence = view->indexAt(QPoint(view->viewport()->rect().left(), y));
                        if (itemProvidesConfidence.isValid())
                        {
                            view->scrollTo(itemProvidesConfidence);
                        }
                    }
                }
                if (keyEvent->key() == Qt::Key_Up)
                {
                    int y = view->visualRect(view->currentIndex()).top() - 49 * 5;
                    if (y < view->viewport()->rect().top())
                    {
                        QModelIndex itemProvidesConfidence = view->indexAt(QPoint(view->viewport()->rect().left(), y));
                        if (itemProvidesConfidence.isValid())
                        {
                            view->scrollTo(itemProvidesConfidence);
                        }
                    }
                }
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

    QSignalMapper* focusFirstItemMapper;
    QSignalMapper* scrollToCurrentItemMapper;

private slots:
    void setMoviesViewRootIndex()
    {
        this->moviesView->setRootIndex(this->moviesModel->rootIndex());
    }

    void focusFirstItem(QObject* object)
    {
        auto view = qobject_cast<QAbstractItemView*>(object);
        view->setCurrentIndex(view->model()->index(0, 0, view->rootIndex()));
    }

    void scrollToCurrentItem(QObject* object)
    {
        auto view = qobject_cast<QAbstractItemView*>(object);
        view->scrollTo(view->currentIndex());
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
        Q_UNUSED(mplayer);
    }
};

#endif // MAINWINDOW_H
