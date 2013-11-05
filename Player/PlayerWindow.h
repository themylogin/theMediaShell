#ifndef PLAYERWINDOW_H
#define PLAYERWINDOW_H

#include <sys/time.h>
#include <sys/resource.h>

#include <algorithm>
#include <cmath>
#include <ctime>

#include <qapplication.h>

#include <QCloseEvent>
#include <QDir>
#include <QFile>
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
#include <QTableWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include <QtConcurrentRun>

#include <QxtGlobalShortcut>

#include <qjson/serializer.h>

#include "Player/PlaylistItem.h"
#include "Player/PlaylistModel.h"
#include "MediaDb.h"

class PlayerWindow : public QWidget
{
    Q_OBJECT

public:
    PlayerWindow(QString playlistName, QString playlistTitle, QStringList playlist, QWidget* parent = 0)
        : QWidget(parent)
    {        
        QFile qss("://Player/PlayerWindow.qss");
        if (qss.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            this->setStyleSheet(qss.readAll());
            qss.close();
        }

        this->playlistName = playlistName;

        this->layout = new QVBoxLayout;
        this->setLayout(layout);

        this->clockLabel = new QLabel(this);
        this->clockLabel->setStyleSheet("QLabel { font: 48px \"Segoe UI\"; margin-top: -5px; margin-bottom: -5px; }");
        this->clockLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        this->layout->addWidget(this->clockLabel);
        this->clockTimer = new QTimer(this);
        this->clockTimer->setInterval(1000);
        connect(this->clockTimer, SIGNAL(timeout()), this, SLOT(updateClockLabel()));
        this->clockTimer->start();
        this->updateClockLabel();

        this->title = playlistTitle;
        this->titleLabel = new QLabel(this);
        this->titleLabel->setText(this->title);
        this->titleLabel->setStyleSheet("QLabel { font: 48px \"Segoe UI\"; margin-top: -5px; margin-bottom: -5px; }");
        this->titleLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        this->layout->addWidget(this->titleLabel);

        // find common prefix among playlist items file names
        QString prefix = playlist[0].split("/").last();
        while (prefix.length() > 0)
        {
            bool ok = true;
            foreach (QString file, playlist)
            {
                QString name = file.split("/").last();
                if (!name.startsWith(prefix))
                {
                    ok = false;
                    break;
                }
            }
            if (ok)
            {
                break;
            }
            else
            {
                prefix = prefix.left(prefix.length() - 1);
            }
        }
        // if prefix ends with digit (e.g. files are S01E16.avi and S01E17.avi,
        // so prefix is 'S01E1' and files are '6.avi' and '7.avi'), shorten it
        // (so prefix will be 'S01E' and files will be '16.avi' and '17.avi')
        while (prefix.length() > 0 && prefix.at(prefix.length() - 1).isDigit())
        {
            prefix = prefix.left(prefix.length() - 1);
        }

        this->playlist = new PlaylistModel(this);
        foreach (QString file, playlist)
        {
            PlaylistItem* item = new PlaylistItem;
            item->title = file.split("/").last();
            if (prefix.length() > 0 && this->playlist->rowCount() > 0)
            {
                item->title = "..." + item->title.right(item->title.length() - prefix.length());
            }
            item->file = file;
            item->duration = nanf("");
            item->isActive = this->playlist->rowCount() == 0;
            this->playlist->addItem(item);
        }
        this->playlistView = new QTableView(this);
        this->playlistView->setModel(this->playlist);
        this->playlistView->horizontalHeader()->hide();
        this->playlistView->verticalHeader()->hide();
        this->playlistView->verticalHeader()->setDefaultSectionSize(49); // TODO: Find a way to put this into stylesheet
        this->playlistView->setShowGrid(false);
        this->playlistView->setColumnWidth(0, 510);
        this->playlistView->setColumnWidth(1, 110);
        this->playlistView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        this->playlistView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        this->playlistView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        this->playlistView->setMaximumHeight(590);
        this->layout->addWidget(this->playlistView, 1);

        this->helpLabel = new QLabel(this);
        this->helpLabel->setText(QString::fromUtf8("Комбинации клавиш"));
        this->helpLabel->setStyleSheet("QLabel { font: 48px \"Segoe UI\"; margin-top: -30px; padding-top: 30px; }");
        this->helpLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        this->layout->addWidget(this->helpLabel);

        this->helpTable = new QTableWidget(this);
        this->helpTable->horizontalHeader()->hide();
        this->helpTable->verticalHeader()->hide();
        this->helpTable->verticalHeader()->setDefaultSectionSize(49); // TODO: Find a way to put this into stylesheet
        this->helpTable->setShowGrid(false);
        this->helpTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        this->helpTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        this->helpTable->setColumnCount(2);
        this->helpTable->setRowCount(5);
        this->helpTable->setColumnWidth(0, 110);
        this->helpTable->setColumnWidth(1, 520);
        this->helpTable->setItem(0, 0, new QTableWidgetItem(QString::fromUtf8("#/j")));
        this->helpTable->setItem(0, 1, new QTableWidgetItem(QString::fromUtf8("Выбор аудио / субтитров")));
        this->helpTable->setItem(1, 0, new QTableWidgetItem(QString::fromUtf8("-/+")));
        this->helpTable->setItem(1, 1, new QTableWidgetItem(QString::fromUtf8("Задержка аудиодорожки")));
        this->helpTable->setItem(2, 0, new QTableWidgetItem(QString::fromUtf8("z/x")));
        this->helpTable->setItem(2, 1, new QTableWidgetItem(QString::fromUtf8("Задержка субтитров")));
        this->helpTable->setItem(3, 0, new QTableWidgetItem(QString::fromUtf8("a/s")));
        this->helpTable->setItem(3, 1, new QTableWidgetItem(QString::fromUtf8("Больше/меньше серий")));
        this->helpTable->setItem(4, 0, new QTableWidgetItem(QString::fromUtf8("w/e")));
        this->openingLength = 0;
        this->endingLength = 0;
        if (this->playlistName != "")
        {
            if (MediaDb::getInstance().contains(this->playlistName, "openingLength"))
            {
                this->openingLength = MediaDb::getInstance().get(this->playlistName, "openingLength").toFloat();
            }
            if (MediaDb::getInstance().contains(this->playlistName, "endingLength"))
            {
                this->endingLength = MediaDb::getInstance().get(this->playlistName, "endingLength").toFloat();
            }
        }
        this->drawOpeningEndingLength();
        this->helpTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        this->helpTable->setFixedSize(620, 238);
        this->layout->addWidget(this->helpTable);

        this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);        
        this->setFixedSize(640, 1080);
        this->QWidget::show();
        this->hide();
        if (this->playlist->rowCount() > 1)
        {
            this->showTemporarily();
        }

        QtConcurrent::run(this, &PlayerWindow::determineDurations, playlist);

        connect(&this->timer, SIGNAL(timeout()), this, SLOT(notifyPlaylist()));
        connect(&this->process, SIGNAL(readyReadStandardOutput()), this, SLOT(readProcessStandardOutput()));
        connect(&this->process, SIGNAL(finished(int)), this, SLOT(processFinished()));
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
        this->openingShortcut = new QxtGlobalShortcut(this);
        this->openingShortcut->setShortcut(QKeySequence("w"));
        connect(this->openingShortcut, SIGNAL(activated()), this, SLOT(openingEndsHere()));
        this->endingShortcut = new QxtGlobalShortcut(this);
        this->endingShortcut->setShortcut(QKeySequence("e"));
        connect(this->endingShortcut, SIGNAL(activated()), this, SLOT(endingStartsHere()));

        this->showTemporarilyTimer.setInterval(5000);
        connect(&this->showTemporarilyTimer, SIGNAL(timeout()), this, SLOT(hide()));
    }

public slots:
    void hide()
    {
        // QApplication::lastWindowClosed() is emited when the last VISIBLE primary window is closed, so this should always be "visible"
        this->move(1920, 0);
        this->showTemporarilyTimer.stop();
    }

    void show()
    {
        this->move(1280, 0);
        this->showTemporarilyTimer.stop();
    }

    void showTemporarily()
    {
        if (!this->isVisible())
        {
            this->show();
            this->showTemporarilyTimer.start();
        }
        else if (this->showTemporarilyTimer.isActive())
        {
            // reload timer if already temporarily shown
            this->showTemporarilyTimer.start();
        }
    }

    bool isVisible() const
    {
        return this->pos().x() == 1280;
    }

protected:
    void closeEvent(QCloseEvent* event)
    {
        foreach (auto hook, this->hooks)
        {
            if (hook->state() != QProcess::NotRunning)
            {
                if (!this->closeTimer.isActive())
                {
                    connect(&this->closeTimer, SIGNAL(timeout()), this, SLOT(close()));
                    this->closeTimer.start(100);
                }
                event->ignore();                
                this->hide();
                return;
            }
        }

        this->closeTimer.stop();
        event->accept();

        delete this->toggleShortcut;
        delete this->planLessShortcut;
        delete this->planMoreShortcut;
        delete this->openingShortcut;
        delete this->endingShortcut;
    }

private:
    QString playlistName;

    QVBoxLayout* layout;

    QLabel* clockLabel;
    QTimer* clockTimer;

    QString title;
    QLabel* titleLabel;

    PlaylistModel* playlist;
    QTableView* playlistView;

    QLabel* helpLabel;
    QTableWidget* helpTable;

    QTimer timer;
    QProcess process;
    QDateTime startedAt;
    float progress;
    float duration;
    QMap<time_t, float> time2progress;

    QxtGlobalShortcut* toggleShortcut;
    QxtGlobalShortcut* planLessShortcut;
    QxtGlobalShortcut* planMoreShortcut;    
    QxtGlobalShortcut* openingShortcut;
    QxtGlobalShortcut* endingShortcut;

    QString openingEndingKey;
    float openingLength;
    float endingLength;

    QList<QProcess*> hooks;

    QTimer showTemporarilyTimer;
    QTimer closeTimer;

    bool findDuration(QString stdout, float& duration)
    {
        QRegExp rx("ID_LENGTH=([0-9]+)");
        if (rx.lastIndexIn(stdout) != -1)
        {
            duration = rx.capturedTexts()[1].toFloat();
            return true;
        }
        else
        {
            return false;
        }
    }

    void determineDurations(QStringList playlist)
    {
        foreach (QString file, playlist)
        {
            float duration = 0;
            if (MediaDb::getInstance().contains(file, "duration"))
            {
                duration = MediaDb::getInstance().get(file, "duration").toFloat();
            }
            else
            {
                QProcess mplayer;
                mplayer.start("mplayer", QStringList() << "-ao" << "null"
                                                       << "-vc" << ","
                                                       << "-vo" << "null"
                                                       << "-frames" << "0"
                                                       << "-identify" << file);
                setpriority(PRIO_PROCESS, mplayer.pid(), 19);
                mplayer.waitForFinished(-1);

                QString stdout = QString::fromUtf8(mplayer.readAllStandardOutput());
                if (this->findDuration(stdout, duration))
                {
                    MediaDb::getInstance().set(file, "duration", duration);
                }
            }
            this->playlist->setDurationFor(file, duration);
        }
    }

    void play()
    {
        if (this->playlist->getFrontItem() && this->playlist->getFrontItem()->isActive)
        {
            QString file = this->playlist->getFrontItem()->file;

            QStringList arguments;
            if (this->openingLength > 0)
            {
                arguments.append("-ss");
                arguments.append(QString::number(this->openingLength));
            }
            else if (MediaDb::getInstance().contains(file, "progress"))
            {
                float progress = MediaDb::getInstance().get(file, "progress").toFloat();
                float duration = MediaDb::getInstance().get(file, "duration").toFloat();
                if (progress / duration < 0.85)
                {
                    arguments.append("-ss");
                    arguments.append(QString::number(progress));
                }
            }
            arguments.append("-identify");
            arguments.append("-slave");
            arguments.append(file);

            this->startedAt = QDateTime::currentDateTime();
            this->process.start("mplayer", arguments);
        }
        else
        {
            this->close();
        }
    }

private slots:
    void updateClockLabel()
    {
        this->clockLabel->setText(QDateTime::currentDateTime().toString("hh:mm"));
    }

    void notifyPlaylist()
    {
        this->playlist->notify(QDateTime::currentDateTime(), this->progress);
    }

    void drawOpeningEndingLength()
    {
        this->helpTable->setItem(4, 1, new QTableWidgetItem(QString::fromUtf8("Опенинг (%1 с.) / титры (%2 с.)").arg((int)this->openingLength).arg((int)this->endingLength)));
    }

    void readProcessStandardOutput()
    {
        QString data = QString::fromUtf8(this->process.readAllStandardOutput());

        float duration;
        if (this->findDuration(data, duration))
        {
            this->duration = duration;
        }

        QRegExp rx("^A:([0-9. ]+)");
        if (rx.lastIndexIn(data) != -1)
        {
            this->progress = rx.capturedTexts()[1].toFloat();
            this->time2progress[time(NULL)] = this->progress;

            if (this->duration - this->progress < this->endingLength)
            {
                this->process.write("quit\n");
            }
        }
    }

    void processFinished()
    {
        auto finishedPlaylistItem = this->playlist->getFrontItem();

        QList<QVariant> pauses;
        int previous_unpaused_time = -1;
        foreach (auto time, this->time2progress.keys())
        {
            if (previous_unpaused_time != -1)
            {
                const int SIGNIFICANT_PAUSE_LENGTH = 10;
                if (time - previous_unpaused_time >= SIGNIFICANT_PAUSE_LENGTH)
                {
                    QVariantMap pause;
                    pause["start"] = (int)previous_unpaused_time;
                    pause["end"] = (int)time;
                    pauses.append(pause);
                }
            }

            previous_unpaused_time = time;
        }

        QVariantMap json;
        json["start"] = this->startedAt.toTime_t();
        json["end"] = QDateTime::currentDateTime().toTime_t();
        json["pauses"] = pauses;
        json["progress"] = this->progress;
        json["duration"] = finishedPlaylistItem->duration;

        QProcess* hook = new QProcess;
        hook->start(QFileInfo(qApp->argv()[0]).absoluteDir().absolutePath() + "/hooks/post-mplayer",
                    QStringList() << finishedPlaylistItem->file);
        QJson::Serializer serializer;
        hook->write(serializer.serialize(json));
        hook->closeWriteChannel();
        this->hooks.append(hook);

        MediaDb::getInstance().set(finishedPlaylistItem->file, "progress", this->progress);

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
        this->showTemporarily();
    }

    void planMore()
    {
        this->playlist->setActiveCount(std::min(this->playlist->activeCount() + 1, this->playlist->rowCount()));
        this->showTemporarily();
    }

    void openingEndsHere()
    {
        this->openingLength = this->progress;
        if (this->playlistName != "")
        {
            MediaDb::getInstance().set(this->playlistName, "openingLength", this->openingLength);
        }
        this->drawOpeningEndingLength();
    }

    void endingStartsHere()
    {
        this->endingLength = this->duration - this->progress;
        if (this->playlistName != "")
        {
            MediaDb::getInstance().set(this->playlistName, "endingLength", this->endingLength);
        }
        this->drawOpeningEndingLength();
    }
};

#endif // PLAYERWINDOW_H
