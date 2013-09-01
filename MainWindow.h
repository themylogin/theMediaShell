#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QEvent>
#include <QFileSystemModel>
#include <QHeaderView>
#include <QKeyEvent>
#include <QMainWindow>
#include <QSignalMapper>
#include <QStatusBar>
#include <QStringList>
#include <QTableView>
#include <QTabWidget>
#include <QTimer>
#include <QTreeView>

#include "Classificator/Classificators.h"
#include "MediaHandler/MediaHandler.h"
#include "MediaModel/MediaModel.h"
#include "MediaModel/NewMediaModel.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    MainWindow(QWidget* parent = 0)
        : QMainWindow(parent)
    {
        QFile qss("://MainWindow.qss");
        if (qss.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            this->setStyleSheet(qss.readAll());
            qss.close();
        }

        this->tabWidget = new QTabWidget(this);
        this->tabWidget->setFocusPolicy(Qt::NoFocus);
        this->setCentralWidget(tabWidget);

        this->keepFirstItemFocusedWhileLoadingMapper = new QSignalMapper(this);
        connect(this->keepFirstItemFocusedWhileLoadingMapper, SIGNAL(mapped(QObject*)), this, SLOT(focusFirstItem(QObject*)));
        this->scrollToCurrentItemMapper = new QSignalMapper(this);
        connect(this->scrollToCurrentItemMapper, SIGNAL(mapped(QObject*)), this, SLOT(scrollToCurrentItem(QObject*)));
    }

    void addTable(QString title, QAbstractItemModel* model, MediaHandler* handler)
    {
        auto view = new QTableView(this->tabWidget);
        view->setModel(model);

        view->setSortingEnabled(true);
        view->horizontalHeader()->hide();
        view->verticalHeader()->hide();
        view->verticalHeader()->setDefaultSectionSize(49); // TODO: Find a way to put this into stylesheet
        view->setSelectionBehavior(QAbstractItemView::SelectRows);
        view->setShowGrid(false);
        this->suitUpView(view, handler);
        // TODO: because of margin in stylesheet, this is awful
        view->setColumnWidth(0, view->columnWidth(0) - 20);
        view->setColumnWidth(2, view->columnWidth(2) + 20);

        this->tabWidget->addTab(view, title);
    }

    void addTree(QString title, QAbstractItemModel* model, const QModelIndex& rootIndex, MediaHandler* handler)
    {
        auto view = new QTreeView(this->tabWidget);
        view->setModel(model);
        view->setRootIndex(rootIndex);

        view->setHeaderHidden(true);
        this->suitUpView(view, handler);

        this->tabWidget->addTab(view, title);
    }
    
protected:
    template<typename T>
    void suitUpView(T* view, MediaHandler* handler)
    {
        view->setColumnWidth(0, 1445);  // Name
        view->setColumnWidth(1, 175);   // Size
        view->setColumnWidth(2, 270);   // Date Modified

        view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        view->setIconSize(QSize(32, 32));

        view->installEventFilter(this);

        this->keepFirstItemFocusedWhileLoadingMapper->setMapping((QObject*)view->model(), (QObject*)view);
        connect(view->model(), SIGNAL(layoutChanged()), this->keepFirstItemFocusedWhileLoadingMapper, SLOT(map()));
        this->scrollToCurrentItemMapper->setMapping((QObject*)view->model(), (QObject*)view);
        connect(view->model(), SIGNAL(layoutChanged()), this->scrollToCurrentItemMapper, SLOT(map()));

        this->handlers[view] = handler;
        connect(view, SIGNAL(activated(QModelIndex)), this, SLOT(mediaActivated(QModelIndex)));
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

        auto view = qobject_cast<QAbstractItemView*>(object);
        if (view)
        {
            if (event->type() == QEvent::KeyPress)
            {
                this->keepFirstItemFocusedWhileLoadingMapper->removeMappings(view->model());

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

                if (keyEvent->key() == Qt::Key_Left)
                {
                    auto treeView = qobject_cast<QTreeView*>(object);
                    if (treeView)
                    {
                        auto parentIndex = treeView->currentIndex().parent();
                        if (parentIndex.isValid())
                        {
                            treeView->collapse(parentIndex);
                            treeView->setCurrentIndex(parentIndex);
                        }
                    }
                }
            }
        }

        return false;
    }

private:
    QTabWidget* tabWidget;

    QSignalMapper* keepFirstItemFocusedWhileLoadingMapper;
    QSignalMapper* scrollToCurrentItemMapper;

    QMap<QAbstractItemView*, MediaHandler*> handlers;

private slots:
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

    void mediaActivated(QModelIndex movie)
    {
        auto view = qobject_cast<QAbstractItemView*>(QObject::sender());

        auto treeView = qobject_cast<QTreeView*>(view);
        if (treeView && treeView->model()->hasChildren(movie))
        {
            if (treeView->isExpanded(movie))
            {
                treeView->collapse(movie);
            }
            else
            {
                treeView->expand(movie);
            }
        }
        else
        {
            this->handlers[view]->activate(movie);
        }
    }

    void updateStatusBar()
    {
        /*
        if (this->tabWidget->currentIndex() == 0)
        {
            QModelIndex selectedIndex = this->moviesView->selectionModel()->currentIndex();
            if (selectedIndex.isValid())
            {
                this->updateStatusBarWithMoviesModelIndex(selectedIndex);
                return;
            }
        }

        if (this->tabWidget->currentIndex() == 1)
        {
            QModelIndex selectedIndex = this->newMoviesView->selectionModel()->currentIndex();
            if (selectedIndex.isValid())
            {
                this->updateStatusBarWithMoviesModelIndex(this->newMoviesModel->mediaModelIndex(selectedIndex));
                return;
            }
        }
        */

        this->statusBar()->hide();
    }

    void updateStatusBarWithMoviesModelIndex(const QModelIndex& index)
    {
        /*
        VideoIdentification identification;
        if (this->moviesModel->identification(index, identification))
        {
            QStringList message;

            if (identification.subtitles.count())
            {
                message << QString::fromUtf8("Субтитры: %1").arg(identification.subtitles.join(", "));
            }

            if (identification.abandonedSubtitles.count())
            {
                message << QString::fromUtf8("Неопознанные субтитры: %1").arg(identification.abandonedSubtitles.count());
            }

            this->statusBar()->show();
            this->statusBar()->showMessage(message.join(" | "));
        }
        else
        {
            this->statusBar()->hide();
        }
        */
    }
};

#endif // MAINWINDOW_H
