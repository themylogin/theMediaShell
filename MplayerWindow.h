#ifndef MPLAYERWINDOW_H
#define MPLAYERWINDOW_H

#include <algorithm>
#include <cmath>

#include <qapplication.h>

#include <QDir>
#include <QFileInfo>
#include <QFont>
#include <QLabel>
#include <QList>
#include <QHeaderView>
#include <QPair>
#include <QProcess>
#include <QRegExp>
#include <QStringList>
#include <QTableView>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include <QtConcurrentRun>

#include <QxtGlobalShortcut>

#include "PlaylistItem.h"
#include "PlaylistModel.h"

class MplayerWindow : public QWidget
{
    Q_OBJECT

public:
    MplayerWindow(QString title, QStringList playlist, QWidget* parent = 0)
        : QWidget(parent)
    {
        this->setStyleSheet(R"(
            *
            {
                background-color: rgb(0, 138, 0);
                color: rgb(240, 241, 240);
                font: 26pt "Segoe UI";
            }

            QTableView
            {
                border: none;
                outline: none;
            }
            QTableView::item:!enabled
            {
                color: rgb(192, 192, 192);
            }
        )");

        this->layout = new QVBoxLayout;
        this->setLayout(layout);

        this->title = title;
        this->titleLabel = new QLabel(this);
        this->titleLabel->setText(this->title);
        this->titleLabel->setStyleSheet("QLabel { font: 36pt \"Segoe UI\"; }");
        this->layout->addWidget(this->titleLabel);

        this->playlist = new PlaylistModel(this);
        foreach (QString file, playlist)
        {
            PlaylistItem* item = new PlaylistItem;
            item->file = file;
            item->duration = nanf("");
            item->isActive = true;
            this->playlist->addItem(item);
        }
        this->playlistView = new QTableView(this);
        this->playlistView->setModel(this->playlist);
        this->playlistView->horizontalHeader()->hide();
        this->playlistView->verticalHeader()->hide();
        this->playlistView->verticalHeader()->setDefaultSectionSize(49); // TODO: Find a way to put this into stylesheet
        this->playlistView->setShowGrid(false);
        this->playlistView->setColumnWidth(0, 530);
        this->playlistView->setColumnWidth(1, 90);
        this->playlistView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        this->playlistView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        this->layout->addWidget(this->playlistView);

        this->layout->addStretch();
        this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);        
        this->setFixedSize(640, 1080);
        this->move(1280, 0);
        this->show();

        QtConcurrent::run(this, &MplayerWindow::determineDurations, playlist);

        connect(&this->timer, SIGNAL(timeout()), this, SLOT(notifyPlaylist()));
        connect(&this->process, SIGNAL(readyReadStandardOutput()), this, SLOT(readProcessStandardOutput()));
        connect(&this->process, SIGNAL(finished(int)), this, SLOT(playNext()));
        this->progress = 0;

        this->timer.start(1000);
        this->play();

        this->toggleShortcut = new QxtGlobalShortcut(this);
        this->toggleShortcut->setShortcut(QKeySequence("o"));
        connect(this->toggleShortcut, SIGNAL(activated()), this, SLOT(toggle()));
        this->planLessShortcut = new QxtGlobalShortcut(this);
        this->planLessShortcut->setShortcut(QKeySequence("a"));
        connect(this->planLessShortcut, SIGNAL(activated()), this, SLOT(planLess()));
        this->planMoreShortcut = new QxtGlobalShortcut(this);
        this->planMoreShortcut->setShortcut(QKeySequence("s"));
        connect(this->planMoreShortcut, SIGNAL(activated()), this, SLOT(planMore()));
    }

private:
    QVBoxLayout* layout;

    QString title;
    QLabel* titleLabel;

    PlaylistModel* playlist;
    QTableView* playlistView;

    QTimer timer;
    QProcess process;
    QDateTime startedAt;
    float progress;

    QxtGlobalShortcut* toggleShortcut;
    QxtGlobalShortcut* planLessShortcut;
    QxtGlobalShortcut* planMoreShortcut;

    void determineDurations(QStringList playlist)
    {
        foreach (QString file, playlist)
        {
            QProcess mplayer;
            mplayer.start("mplayer", QStringList() << "-ao" << "null"
                                                   << "-vc" << ","
                                                   << "-vo" << "null"
                                                   << "-frames" << "0"
                                                   << "-identify" << file);
            mplayer.waitForFinished(-1);

            QString data = QString::fromUtf8(mplayer.readAllStandardOutput());
            QRegExp rx("ID_LENGTH=([0-9]+)");
            if (rx.lastIndexIn(data) != -1)
            {
                this->playlist->setDurationFor(file, rx.capturedTexts()[1].toFloat());
            }
        }
    }

    void play()
    {
        if (this->playlist->getFrontItem() && this->playlist->getFrontItem()->isActive)
        {
            this->startedAt = QDateTime::currentDateTime();
            this->process.start("mplayer", QStringList() << this->playlist->getFrontItem()->file);
        }
        else
        {
            this->close();
        }
    }

private slots:
    void notifyPlaylist()
    {
        this->playlist->notify(QDateTime::currentDateTime(), this->progress);
    }

    void readProcessStandardOutput()
    {
        QString data = QString::fromUtf8(process.readAllStandardOutput());
        QRegExp rx("^A:([0-9. ]+)");
        if (rx.lastIndexIn(data) != -1)
        {
            this->progress = rx.capturedTexts()[1].toFloat();
        }
    }

    void playNext()
    {
        QProcess* hook = new QProcess;
        hook->start(QFileInfo(qApp->argv()[0]).absoluteDir().absolutePath() + "/hooks/post-mplayer",
                    QStringList() << this->playlist->getFrontItem()->file
                                  << QString::number(this->startedAt.toTime_t())
                                  << QString::number(QDateTime::currentDateTime().toTime_t()));
        this->playlist->popFrontItem();
        this->play();
    }

    void toggle()
    {
        if (this->isVisible())
        {
            this->hide();
        }
        else
        {
            this->show();
        }
    }

    void planLess()
    {
        this->playlist->setActiveCount(std::max(this->playlist->activeCount() - 1, 1));
    }

    void planMore()
    {
        this->playlist->setActiveCount(std::min(this->playlist->activeCount() + 1, this->playlist->rowCount()));
    }
};

#endif // MPLAYERWINDOW_H
