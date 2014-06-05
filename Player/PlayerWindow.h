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
#include <QHBoxLayout>
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

#include "mpv/client.h"

#include "Player/PlaylistItem.h"
#include "Player/PlaylistModel.h"
#include "MediaDb.h"
#include "Utils.h"

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

        this->clockLayout = new QHBoxLayout;
        this->clockLabel = new QLabel(this);
        this->clockLabel->setStyleSheet("QLabel { font: 48px \"Segoe UI\"; margin-top: -5px; margin-bottom: -5px; }");
        this->clockLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        this->clockLayout->addWidget(this->clockLabel);
        this->clockTimer = new QTimer(this);
        this->clockTimer->setInterval(1000);
        connect(this->clockTimer, SIGNAL(timeout()), this, SLOT(updateClockLabel()));
        this->clockTimer->start();
        this->updateClockLabel();
        this->powerLabel = new QLabel(this);
        this->powerLabel->setPixmap(QPixmap::fromImage(QImage("://power.png")));
        this->clockLayout->addWidget(this->powerLabel);
        this->powerOffOnFinish = false;
        this->updatePowerLabel();
        this->clockLayout->addStretch();
        this->layout->addLayout(this->clockLayout);

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

        this->mpv = NULL;
        this->progress = 0;
        this->duration = 0;
        this->remaining = 0;
        connect(&this->timer, SIGNAL(timeout()), this, SLOT(checkEvents()));

        this->timer.start(100);
        this->play();

        this->stopShortcut = new QxtGlobalShortcut(this);
        this->stopShortcut->setShortcut(QKeySequence("q"));
        connect(this->stopShortcut, SIGNAL(activated()), this, SLOT(stop()));
        this->toggleShortcut = new QxtGlobalShortcut(this);
        this->toggleShortcut->setShortcut(QKeySequence("o"));
        connect(this->toggleShortcut, SIGNAL(activated()), this, SLOT(toggle()));
        this->powerOffOnFinishShortcut = new QxtGlobalShortcut(this);
        this->powerOffOnFinishShortcut->setShortcut(QKeySequence("p"));
        connect(this->powerOffOnFinishShortcut, SIGNAL(activated()), this, SLOT(togglePowerOffOnFinish()));
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

        delete this->stopShortcut;
        delete this->toggleShortcut;
        delete this->powerOffOnFinishShortcut;
        delete this->planLessShortcut;
        delete this->planMoreShortcut;
        delete this->openingShortcut;
        delete this->endingShortcut;
    }

private:
    QString playlistName;

    QVBoxLayout* layout;

    QHBoxLayout* clockLayout;
    QLabel* clockLabel;
    QTimer* clockTimer;
    QLabel* powerLabel;

    bool powerOffOnFinish;

    QString title;
    QLabel* titleLabel;

    PlaylistModel* playlist;
    QTableView* playlistView;

    QLabel* helpLabel;
    QTableWidget* helpTable;

    QTimer timer;
    mpv_handle* mpv;

    double progress;
    double duration;
    double remaining;

    QxtGlobalShortcut* stopShortcut;
    QxtGlobalShortcut* toggleShortcut;
    QxtGlobalShortcut* powerOffOnFinishShortcut;
    QxtGlobalShortcut* planLessShortcut;
    QxtGlobalShortcut* planMoreShortcut;    
    QxtGlobalShortcut* openingShortcut;
    QxtGlobalShortcut* endingShortcut;

    QString openingEndingKey;
    double openingLength;
    double endingLength;

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
                QProcess mpv;
                mpv.start("mpv", QStringList() << "-frames" << "0"
                                               << "-identify" << file);
                setpriority(PRIO_PROCESS, mpv.pid(), 19);
                mpv.waitForFinished(-1);

                QString stdout = QString::fromUtf8(mpv.readAllStandardOutput());
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

            this->mpv = mpv_create();
            mpv_set_option_string(this->mpv, "input-default-bindings", "yes");

            mpv_set_option_string(this->mpv, "vo", "vdpau");
            mpv_set_option_string(this->mpv, "hwdec", "vdpau");

            mpv_set_option_string(this->mpv, "fs", "yes");

            mpv_set_option_string(this->mpv, "channels", "2");
            mpv_set_option_string(this->mpv, "alang", "ja,jp,jpn,en,eng,ru,rus");

            mpv_set_option_string(this->mpv, "autosub-match", "fuzzy");
            mpv_set_option_string(this->mpv, "slang", "ru,rus,en,eng");
            mpv_set_option_string(this->mpv, "subcp", "enca:ru");

            mpv_initialize(this->mpv);

            mpv_observe_property(this->mpv, 0, "length", MPV_FORMAT_DOUBLE);
            mpv_observe_property(this->mpv, 0, "aid", MPV_FORMAT_STRING);

            auto fileUtf8 = file.toUtf8();
            const char* loadCmd[] = {"loadfile", fileUtf8.constData(), NULL};
            mpv_command(this->mpv, loadCmd);

            mpv_set_option_string(this->mpv, "idle", "no");

            if (MediaDb::getInstance().contains(file, "progress"))
            {
                float progress = MediaDb::getInstance().get(file, "progress").toFloat();
                float duration = MediaDb::getInstance().get(file, "duration").toFloat();
                if (!Utils::isWatched(progress, duration))
                {
                    auto secondsUtf8 = QString::number(progress).toUtf8();
                    const char* seekCmd[] = {"seek", secondsUtf8.constData(), "absolute", NULL};
                    mpv_command(this->mpv, seekCmd);
                }
            }
            else if (this->openingLength > 0)
            {
                auto secondsUtf8 = QString::number(this->openingLength).toUtf8();
                const char* seekCmd[] = {"seek", secondsUtf8.constData(), "absolute", NULL};
                mpv_command(this->mpv, seekCmd);
            }

            if (this->playlistName != "")
            {
                if (MediaDb::getInstance().contains(this->playlistName, "aid"))
                {
                    mpv_set_property_string(this->mpv, "aid", MediaDb::getInstance().get(this->playlistName, "aid").toString().toUtf8().constData());
                }
            }

            this->progress = 0;
            this->duration = 0;
            this->remaining = 0;
        }
        else
        {
            if (this->powerOffOnFinish)
            {
                QProcess* hook = new QProcess;
                hook->start(this->getHookPath("power-off"));
                hook->closeWriteChannel();
                this->hooks.append(hook);
            }

            this->close();
        }
    }

    QString getHookPath(QString hookName)
    {
        return QFileInfo(qApp->argv()[0]).absoluteDir().absolutePath() + "/hooks/" + hookName;
    }

private slots:
    void updateClockLabel()
    {
        this->clockLabel->setText(QDateTime::currentDateTime().toString("hh:mm"));
    }

    void updatePowerLabel()
    {
        this->powerLabel->setVisible(this->powerOffOnFinish);
    }

    void togglePowerOffOnFinish()
    {
        this->powerOffOnFinish = !this->powerOffOnFinish;
        this->updatePowerLabel();
        this->showTemporarily();
    }

    void checkEvents()
    {
        if (this->mpv != NULL)
        {
            mpv_event* event = mpv_wait_event(this->mpv, 0);

            if (event->event_id == MPV_EVENT_SHUTDOWN)
            {
                mpv_destroy(this->mpv);
                this->mpv = NULL;

                auto finishedPlaylistItem = this->playlist->getFrontItem();

                MediaDb::getInstance().set(finishedPlaylistItem->file, "progress", this->progress);

                this->playlist->popFrontItem();
                this->play();
                return;
            }

            if (event->event_id == MPV_EVENT_PROPERTY_CHANGE)
            {
                mpv_event_property* property_event = (mpv_event_property*)event->data;

                if (property_event->name == QLatin1String("length"))
                {
                    this->duration = *((double*)property_event->data);
                }

                if (property_event->name == QLatin1String("aid"))
                {
                    if (this->playlistName != "")
                    {
                        MediaDb::getInstance().set(this->playlistName, "aid", QString::fromUtf8(*((char**)property_event->data)));
                    }
                }
            }

            mpv_get_property(this->mpv, "time-pos", MPV_FORMAT_DOUBLE, &this->progress);
            mpv_get_property(this->mpv, "playtime-remaining", MPV_FORMAT_DOUBLE, &this->remaining);

            if (this->duration > 0 &&
                this->progress > 0 &&
                this->duration - this->progress < this->endingLength)
            {
                const char* quitCmd[] = {"quit", NULL};
                mpv_command(this->mpv, quitCmd);
            }

            this->playlist->notify(QDateTime::currentDateTime(), this->remaining);
        }
    }

    void drawOpeningEndingLength()
    {
        this->helpTable->setItem(4, 1, new QTableWidgetItem(QString::fromUtf8("Опенинг (%1 с.) / титры (%2 с.)").arg((int)this->openingLength).arg((int)this->endingLength)));
    }

    void stop()
    {
        this->powerOffOnFinish = false; // manual interruption is not finish
        this->playlist->setActiveCount(1);

        const char* quitCmd[] = {"quit", NULL};
        mpv_command(this->mpv, quitCmd);
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
        if (this->playlistName != "")
        {
            this->openingLength = this->progress;
            MediaDb::getInstance().set(this->playlistName, "openingLength", this->openingLength);
            this->drawOpeningEndingLength();
            this->showTemporarily();
        }
    }

    void endingStartsHere()
    {
        if (this->playlistName != "")
        {
            this->endingLength = this->duration - this->progress;
            MediaDb::getInstance().set(this->playlistName, "endingLength", this->endingLength);
            this->drawOpeningEndingLength();
            this->showTemporarily();
        }
    }
};

#endif // PLAYERWINDOW_H
