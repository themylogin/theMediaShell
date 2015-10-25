#include "PlayerWindow.h"

#include <sys/resource.h>
#include <sys/time.h>

#include <algorithm>
#include <cmath>
#include <ctime>
#include <iostream>

#include <QCloseEvent>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFont>
#include <QList>
#include <QHeaderView>
#include <QPair>
#include <QProcess>
#include <QSettings>
#include <QStringList>
#include <QTimer>
#include <QWidget>

#include <QtConcurrent/QtConcurrentRun>

#include "Hook.h"
#include "Player/PlaylistItem.h"
#include "MediaDb.h"
#include "Mpd/MpdDialog.h"
#include "Utils.h"

PlayerWindow::PlayerWindow(QString playlistName, QString playlistTitle, QStringList playlist,
                           MpdClient* mpd, bool* mpdWasPlaying,
                           QWidget* parent)
    : QMainWindow(parent)
{    
    this->playlistName = playlistName;
    this->playlistTitle = playlistTitle;

    QPalette pal = this->palette();
    pal.setColor(QPalette::Background, Qt::black);
    this->setAutoFillBackground(true);
    this->setPalette(pal);

    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->showFullScreen();

    this->initPlayer();

    this->initSidebar(playlistTitle, playlist);
    if (this->playlist->rowCount() > 1)
    {
        this->showSidebarTemporarily();
    }

    this->initThemylog();

    this->mpd = mpd;
    this->mpdWasPlaying = mpdWasPlaying;

    this->play();
}

PlayerWindow::~PlayerWindow()
{
    if (this->mpv)
    {
        mpv_set_option_string(this->mpv, "vid", "no");
        mpv_terminate_destroy(this->mpv);
    }
}

void PlayerWindow::initSidebar(const QString& playlistTitle, const QStringList& playlist)
{
    this->sidebar = new QWidget(this);
    this->sidebar->move(1280, 0);
    this->sidebar->setFixedSize(640, 1080);
    this->sidebar->setFocusPolicy(Qt::NoFocus);
    Utils::setStyleSheetFromFile(this->sidebar, "://Player/PlayerWindowSidebar.qss");

    this->sidebarLayout = new QVBoxLayout;
    this->sidebar->setLayout(this->sidebarLayout);

    this->showSidebarTemporarilyTimer.setInterval(5000);
    connect(&this->showSidebarTemporarilyTimer, SIGNAL(timeout()), this, SLOT(hideSidebar()));

    this->initClock();
    this->initTitle(playlistTitle);
    this->initPlaylist(playlist);
    this->initHelp();
}

void PlayerWindow::initClock()
{
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
    this->sidebarLayout->addLayout(this->clockLayout);
}

void PlayerWindow::updateClockLabel()
{
    this->clockLabel->setText(QDateTime::currentDateTime().toString("hh:mm"));
}

void PlayerWindow::updatePowerLabel()
{
    this->powerLabel->setVisible(this->powerOffOnFinish);
}

void PlayerWindow::togglePowerOffOnFinish()
{
    this->powerOffOnFinish = !this->powerOffOnFinish;
    this->updatePowerLabel();
    this->showSidebarTemporarily();
}

void PlayerWindow::toggleSidebar()
{
    if (this->sidebar->isVisible())
    {
        this->sidebar->hide();
    }
    else
    {
        this->sidebar->show();
    }
}

void PlayerWindow::initTitle(const QString &playlistTitle)
{
    this->title = playlistTitle;
    this->titleLabel = new QLabel(this);
    this->titleLabel->setText(this->title);
    this->titleLabel->setStyleSheet("QLabel { font: 48px \"Segoe UI\"; margin-top: -5px; margin-bottom: -5px; }");
    this->titleLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    this->sidebarLayout->addWidget(this->titleLabel);
}

void PlayerWindow::initPlaylist(const QStringList& playlist)
{
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
    this->playlistView->setFocusPolicy(Qt::NoFocus);
    this->playlistView->setColumnWidth(0, 510);
    this->playlistView->setColumnWidth(1, 110);
    this->playlistView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->playlistView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->playlistView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->playlistView->setMaximumHeight(590);
    this->sidebarLayout->addWidget(this->playlistView, 1);

    QtConcurrent::run(this, &PlayerWindow::determineDurations, playlist);
}

void PlayerWindow::determineDurations(QStringList playlist)
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
            duration = Utils::determineDuration(file);
            if (duration > 0)
            {
                MediaDb::getInstance().set(file, "duration", duration);
            }
        }
        this->playlist->setDurationFor(file, duration);
    }
}

void PlayerWindow::initHelp()
{
    this->helpLabel = new QLabel(this);
    this->helpLabel->setText(QString::fromUtf8("Комбинации клавиш"));
    this->helpLabel->setStyleSheet("QLabel { font: 48px \"Segoe UI\"; margin-top: -30px; padding-top: 30px; }");
    this->helpLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    this->sidebarLayout->addWidget(this->helpLabel);

    this->helpTable = new QTableWidget(this);
    this->helpTable->horizontalHeader()->hide();
    this->helpTable->verticalHeader()->hide();
    this->helpTable->verticalHeader()->setDefaultSectionSize(49); // TODO: Find a way to put this into stylesheet
    this->helpTable->setShowGrid(false);
    this->helpTable->setFocusPolicy(Qt::NoFocus);
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
    this->initOpeningEnding();
    this->helpTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->helpTable->setFixedSize(620, 238);
    this->sidebarLayout->addWidget(this->helpTable);

    this->hotkeys[Qt::Key_Q] = std::bind(&PlayerWindow::stop, this);
    this->hotkeys[Qt::Key_W] = std::bind(&PlayerWindow::openingEndsHere, this);
    this->hotkeys[Qt::Key_E] = std::bind(&PlayerWindow::endingStartsHere, this);
    this->hotkeys[Qt::Key_Y] = std::bind(&PlayerWindow::toggleUpscale, this);
    this->hotkeys[Qt::Key_O] = std::bind(&PlayerWindow::toggleSidebar, this);
    this->hotkeys[Qt::Key_P] = std::bind(&PlayerWindow::togglePowerOffOnFinish, this);
    this->hotkeys[Qt::Key_A] = std::bind(&PlayerWindow::planLess, this);
    this->hotkeys[Qt::Key_S] = std::bind(&PlayerWindow::planMore, this);
    this->hotkeys[Qt::Key_Space] = std::bind(&PlayerWindow::togglePause, this);
    this->hotkeys[Qt::Key_Print] = std::bind(&PlayerWindow::takeScreenshot, this);
    this->hotkeys[Qt::Key_Pause] = std::bind(&PlayerWindow::tweet, this);
}

void PlayerWindow::initOpeningEnding()
{
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
}

void PlayerWindow::drawOpeningEndingLength()
{
    this->helpTable->setItem(4, 1, new QTableWidgetItem(QString::fromUtf8("Опенинг (%1 с.) / титры (%2 с.)").arg((int)this->openingLength).arg((int)this->endingLength)));
}

void PlayerWindow::initPlayer()
{
    QPalette pal;

    this->mpvContainer = new QWidget(this);
    pal = this->mpvContainer->palette();
    pal.setColor(QPalette::Background, Qt::black);
    this->mpvContainer->setAutoFillBackground(true);
    this->mpvContainer->setPalette(pal);
    this->mpvContainer->setFixedSize(1920, 1080);
    this->mpvContainer->show();

    /* From MPV documentation:
     * On X11, a sub-window with input enabled grabs all keyboard input as long as it is:
     * 1. a child of a focused window, and
     * 2. the mouse is inside of the sub-window.
     * This can steal away all keyboard input from the application embedding the mpv window,
     * and on the other hand, the mpv window will receive no input if the mouse is outside of
     * the mpv window, even though mpv has focus. Modern toolkits work around this weird X11
     * behavior, but naively embedding foreign windows breaks it.
     * The only way to handle this reasonably is using the XEmbed protocol, which was designed
     * to solve these problems. But Qt has questionable support, and mpv doesn't implement it yet.
     *
     * We work around this bug by putting a black pixel window to the top left corner
     * of the screen and periodically moving mouse up there.
     */
    this->mpvContainerPseudoFocusDistracter = new QWidget(this);
    pal = this->mpvContainerPseudoFocusDistracter->palette();
    pal.setColor(QPalette::Background, Qt::black);
    this->mpvContainerPseudoFocusDistracter->setAutoFillBackground(true);
    this->mpvContainerPseudoFocusDistracter->setPalette(pal);
    this->mpvContainerPseudoFocusDistracter->setCursor(Qt::BlankCursor);
    this->mpvContainerPseudoFocusDistracter->setFixedSize(1, 1);
    this->mpvContainerPseudoFocusDistracter->move(0, 0);
    this->mpvContainerPseudoFocusDistracter->show();

    this->mpvContainerPseudoFocusDistracterTimer = new QTimer(this);
    connect(this->mpvContainerPseudoFocusDistracterTimer, SIGNAL(timeout()), this, SLOT(distractFocusFromMpvContainer()));
    this->mpvContainerPseudoFocusDistracterTimer->start(100);

    this->mpv = NULL;
    this->resetPlayerProperties();

    QSettings settings;
    this->upscale = settings.value("upscale", true).toBool();
}

void PlayerWindow::resizeMpvContainer()
{
    if (this->width > 0 && this->height > 0)
    {
        QSettings settings;
        QPoint screenCenter = settings.value("screenCenter", QPoint(1920 / 2, 1080 / 2)).toPoint();

        int64_t width = this->width;
        int64_t height = this->height;

        if (this->upscale)
        {
            if ((double)width / height < 1920.0 / 1080)
            {
                width *= 1080.0 / height;
                height = 1080;
            }
            else
            {
                height *= 1920.0 / width;
                width = 1920;
            }
        }
        else
        {
            QSize minimumImageSize = settings.value("minimumImageSize", QSize(0, 0)).toSize();
            
            if (minimumImageSize.width() > 0 && width < minimumImageSize.width())
            {
                height *= (double)minimumImageSize.width() / width;
                width = minimumImageSize.width();
            }
            if (minimumImageSize.height() > 0 && height < minimumImageSize.height())
            {
                width *= (double)minimumImageSize.height() / height;
                height = minimumImageSize.height();
            }

            if (width > 1920)
            {
                height /= (double)width / 1920;
                width = 1920;
            }
            if (height > 1080)
            {
                width /= (double)height / 1080;
                height = 1080;
            }
        }
        
        QPoint topLeft = screenCenter - QPoint(width / 2, height / 2);
        
        if (topLeft.x() < 0)
        {
            topLeft.setX(0);
        }
        else if (topLeft.x() + width > 1920)
        {
            topLeft.setX(1920 - width);
        }

        if (topLeft.y() < 0)
        {
            topLeft.setY(0);
        }
        else if (topLeft.y() + height > 1080)
        {
            topLeft.setY(1080 - height);
        }

        this->mpvContainer->setFixedSize(width, height);
        this->mpvContainer->move(topLeft);
    }
    else
    {
        this->mpvContainer->setFixedSize(1920, 1080);
        this->mpvContainer->move(0, 0);
    }
}

void PlayerWindow::toggleUpscale()
{
    this->upscale = !this->upscale;

    QSettings settings;
    settings.setValue("upscale", this->upscale);

    this->resizeMpvContainer();
}

void PlayerWindow::resetPlayerProperties()
{
    this->paused = false;
    this->progress = 0;
    this->notifiedProgress = 0;
    this->duration = 0;
    this->remaining = 0;
    this->width = 0;
    this->height = 0;
}

void PlayerWindow::initThemylog()
{
    if (getenv("THEMYLOG"))
    {
        auto address = QString::fromLatin1(getenv("THEMYLOG"));
        this->themylogSocket = new QUdpSocket(this);
        this->themylogAddress = QHostAddress(address.split(':')[0]);
        this->themylogPort = address.split(':')[1].toUInt();
    }
    else
    {
        this->themylogSocket = NULL;
    }
}

void PlayerWindow::themylog(const QString& logger, const QString& level, const QString& msg, const QString& movie, const QMap<QString, QString>& args)
{
    if (this->themylogSocket)
    {
        QString data = QString("application=theMediaShell\n"
                               "logger=%1\n"
                               "level=%2\n"
                               "msg=%3\n"
                               "movie=%4").arg(logger)
                                          .arg(level)
                                          .arg(msg)
                                          .arg(movie);
        for (auto iter = args.begin(); iter != args.end(); ++iter)
        {
            data += "\n" + iter.key() + "=" + iter.value();
        }

        this->themylogSocket->writeDatagram(data.toUtf8(), this->themylogAddress, this->themylogPort);
    }
}

void PlayerWindow::themylog(const QString& msg, const QString& movie, const QMap<QString, QString>& args)
{
    this->themylog("movie", "info", msg, movie, args);
}

QMap<QString, QString> PlayerWindow::withProgresDuration(QMap<QString, QString> args)
{
    args["progress"] = QString("%1").arg(this->progress);
    args["duration"] = QString("%1").arg(this->duration);
    return args;
}

QMap<QString, QString> PlayerWindow::withDownloadedAt(QMap<QString, QString> args, QString file)
{
    auto fi = QFileInfo(file);
    args["downloaded_at"] = QString("%1").arg(fi.lastModified().toTime_t());
    return args;
}

void PlayerWindow::showSidebar()
{
    this->sidebar->show();
    this->showSidebarTemporarilyTimer.stop();
}

void PlayerWindow::showSidebarTemporarily()
{
    if (!this->sidebar->isVisible())
    {
        this->showSidebar();
        this->showSidebarTemporarilyTimer.start();
    }
    else if (this->showSidebarTemporarilyTimer.isActive())
    {
        // reload timer if already temporarily shown
        this->showSidebarTemporarilyTimer.start();
    }
}

void PlayerWindow::hideSidebar()
{
    this->sidebar->hide();
    this->showSidebarTemporarilyTimer.stop();
}

bool PlayerWindow::event(QEvent* event)
{
    if (event->type() == QEvent::User)
    {
        while (this->mpv)
        {
            mpv_event* event = mpv_wait_event(this->mpv, 0);
            if (event->event_id != MPV_EVENT_NONE)
            {
                this->handleMpvEvent(event);
            }
            else
            {
                break;
            }
        }

        return true;
    }

    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyEvent = dynamic_cast<QKeyEvent*>(event);
        int key = keyEvent->key();

        if (this->hotkeys.contains(key))
        {
            this->hotkeys[key]();
            return true;
        }

        Utils::x11KeyEventForChildren(this->mpvContainer->winId(), 1, keyEvent->nativeVirtualKey(), keyEvent->nativeModifiers());
        Utils::x11KeyEventForChildren(this->mpvContainer->winId(), 0, keyEvent->nativeVirtualKey(), keyEvent->nativeModifiers());
    }

    return QWidget::event(event);
}

void PlayerWindow::play()
{
    if (this->playlist->getFrontItem() && this->playlist->getFrontItem()->isActive)
    {
        this->file = this->playlist->getFrontItem()->file;

        this->createMpv(this->file);

        this->resetPlayerProperties();

        QMap<QString, QString> args;
        this->themylog("start", this->file, this->withDownloadedAt(args, this->file));
    }
    else
    {
        if (this->powerOffOnFinish)
        {
            Hook::run("power-off");
        }

        emit this->closing(this->powerOffOnFinish);
        this->close();
    }
}

void PlayerWindow::createMpv(const QString& file)
{
    setlocale(LC_NUMERIC, "C");

    this->mpv = mpv_create();

    int64_t wid = this->mpvContainer->winId();
    mpv_set_option(this->mpv, "wid", MPV_FORMAT_INT64, &wid);

    mpv_set_option_string(this->mpv, "input-default-bindings", "yes");
    mpv_set_option_string(this->mpv, "input-vo-keyboard", "yes");

    QString configPath = QStandardPaths::locate(QStandardPaths::HomeLocation, ".mpv/config");
    if (!configPath.isEmpty())
    {
        auto configPathUtf8 = configPath.toUtf8();
        mpv_load_config_file(this->mpv, configPathUtf8.constData());
    }

    mpv_observe_property(this->mpv, 0, "aid", MPV_FORMAT_STRING);
    mpv_observe_property(this->mpv, 0, "length", MPV_FORMAT_DOUBLE);
    mpv_observe_property(this->mpv, 0, "pause", MPV_FORMAT_FLAG);
    mpv_observe_property(this->mpv, 0, "playtime-remaining", MPV_FORMAT_DOUBLE);
    mpv_observe_property(this->mpv, 0, "time-pos", MPV_FORMAT_DOUBLE);

    mpv_request_log_messages(this->mpv, "info");

    mpv_set_wakeup_callback(this->mpv, [](void* ctx)
    {
        QCoreApplication::postEvent((PlayerWindow*)ctx, new QEvent(QEvent::User));
    }, this);

    mpv_initialize(this->mpv);

    if (QFileInfo(file).isDir())
    {
        if (QFileInfo(file + "/BDMV").isDir())
        {
            auto bdFile = "bd:///" + file;
            auto fileUtf8 = bdFile.toUtf8();
            const char* loadCmd[] = {"loadfile", fileUtf8.constData(), NULL};
            mpv_command(this->mpv, loadCmd);
        }
        if (QFileInfo(file + "/VIDEO_TS").isDir())
        {
            auto dvdFile = "dvd:///" + file + "/VIDEO_TS";
            auto fileUtf8 = dvdFile.toUtf8();
            const char* loadCmd[] = {"loadfile", fileUtf8.constData(), NULL};
            mpv_command(this->mpv, loadCmd);
        }
    }
    else
    {
        auto fileUtf8 = file.toUtf8();
        const char* loadCmd[] = {"loadfile", fileUtf8.constData(), NULL};
        mpv_command(this->mpv, loadCmd);
    }

    mpv_set_option_string(this->mpv, "idle", "no");
}

void PlayerWindow::tuneMpv(const QString &file)
{
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
}

void PlayerWindow::handleMpvEvent(mpv_event* event)
{
    if (event->event_id == MPV_EVENT_SHUTDOWN)
    {
        mpv_terminate_destroy(this->mpv);
        this->mpv = NULL;

        auto finishedPlaylistItem = this->playlist->getFrontItem();

        MediaDb::getInstance().set(finishedPlaylistItem->file, "progress", this->progress);

        QMap<QString, QString> args;
        auto file = finishedPlaylistItem->file;
        this->themylog("end", file, this->withDownloadedAt(this->withProgresDuration(args), file));

        this->playlist->popFrontItem();
        this->play();
        return;
    }

    if (event->event_id == MPV_EVENT_LOG_MESSAGE)
    {
        struct mpv_event_log_message* msg = (struct mpv_event_log_message*)event->data;
        std::cerr << msg->level << ": " << msg->text;
        return;
    }

    if (event->event_id == MPV_EVENT_FILE_LOADED)
    {
        this->tuneMpv(this->file);
        return;
    }

    if (event->event_id == MPV_EVENT_VIDEO_RECONFIG)
    {
        int64_t width, height;
        if (mpv_get_property(mpv, "dwidth", MPV_FORMAT_INT64, &width) >= 0 &&
            mpv_get_property(mpv, "dheight", MPV_FORMAT_INT64, &height) >= 0 &&
            width > 0 && height > 0)
        {
            this->width = width;
            this->height = height;
            this->resizeMpvContainer();
        }
    }

    if (event->event_id == MPV_EVENT_PROPERTY_CHANGE)
    {
        mpv_event_property* property_event = (mpv_event_property*)event->data;

        if (property_event->name == QLatin1String("aid"))
        {
            if (this->playlistName != "")
            {
                MediaDb::getInstance().set(this->playlistName, "aid", QString::fromUtf8(*((char**)property_event->data)));
            }
        }

        if (property_event->name == QLatin1String("length"))
        {
            this->duration = *((double*)property_event->data);

            MediaDb::getInstance().set(this->file, "duration", this->duration);
            this->playlist->setDurationFor(this->file, this->duration);
        }

        if (property_event->name == QLatin1String("pause"))
        {
            bool paused = *((int*)property_event->data);
            if (paused != this->paused)
            {
                this->paused = paused;

                QString msg;
                if (this->paused)
                {
                    msg = "pause";
                }
                else
                {
                    msg = "resume";
                }


                QMap<QString, QString> args;
                auto file = this->playlist->getFrontItem()->file;
                this->themylog(msg, file, this->withDownloadedAt(this->withProgresDuration(args), file));
            }
        }

        if (property_event->name == QLatin1String("playtime-remaining"))
        {
            this->remaining = *((double*)property_event->data);
        }

        if (property_event->name == QLatin1String("time-pos"))
        {
            this->progress = *((double*)property_event->data);
        }
    }

    if (this->duration > 0 &&
        this->progress > 0 &&
        this->duration - this->progress < this->endingLength)
    {
        const char* quitCmd[] = {"quit", NULL};
        mpv_command(this->mpv, quitCmd);
    }

    this->playlist->notify(QDateTime::currentDateTime(), this->remaining);

    if (!this->paused)
    {
        if ((int)this->progress != (int)this->notifiedProgress)
        {
            QMap<QString, QString> args;
            auto file = this->playlist->getFrontItem()->file;
            args["remaining"] = QString("%1").arg(this->remaining);
            this->themylog("progress", file, this->withProgresDuration(args));

            this->notifiedProgress = this->progress;
        }
    }
}

void PlayerWindow::distractFocusFromMpvContainer()
{
    if (this->isActiveWindow())
    {
        QCursor::setPos(0, 0);
    }
}

void PlayerWindow::togglePause()
{
    int paused;
    if (mpv_get_property(this->mpv, "pause", MPV_FORMAT_FLAG, &paused) < 0)
    {
        return;
    }

    if (paused)
    {
        bool wasPlaying;
        if (MpdDialog::waitMusicOver(this->playlistTitle, this->mpd, false, &wasPlaying, this))
        {
            paused = 0;
            *this->mpdWasPlaying = *this->mpdWasPlaying | wasPlaying;
            mpv_set_property(this->mpv, "pause", MPV_FORMAT_FLAG, &paused);
        }
    }
    else
    {
        paused = 1;
        mpv_set_property(this->mpv, "pause", MPV_FORMAT_FLAG, &paused);

        // Nobody needs this shit
        // if (*this->mpdWasPlaying)
        // {
        //     this->mpd->resume();
        // }
    }
}

void PlayerWindow::stop()
{
    this->powerOffOnFinish = false; // manual interruption is not finish
    this->playlist->setActiveCount(1);

    const char* quitCmd[] = {"quit", NULL};
    mpv_command(this->mpv, quitCmd);
}

void PlayerWindow::planLess()
{
    this->playlist->setActiveCount(std::max(this->playlist->activeCount() - 1, 1));
    this->showSidebarTemporarily();
}

void PlayerWindow::planMore()
{
    this->playlist->setActiveCount(std::min(this->playlist->activeCount() + 1, this->playlist->rowCount()));
    this->showSidebarTemporarily();
}

void PlayerWindow::openingEndsHere()
{
    if (this->playlistName != "")
    {
        this->openingLength = this->progress;
        MediaDb::getInstance().set(this->playlistName, "openingLength", this->openingLength);
        this->drawOpeningEndingLength();
        this->showSidebarTemporarily();
    }
}

void PlayerWindow::endingStartsHere()
{
    if (this->playlistName != "")
    {
        this->endingLength = this->duration - this->progress;
        MediaDb::getInstance().set(this->playlistName, "endingLength", this->endingLength);
        this->drawOpeningEndingLength();
        this->showSidebarTemporarily();
    }
}

QString PlayerWindow::screenshotsDir()
{
    return QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/screenshots/" + this->file;
}

void PlayerWindow::takeScreenshot()
{
    QString screenshotFileDir = this->screenshotsDir();
    QDir dir(screenshotFileDir);
    if (!dir.exists())
    {
        dir.mkpath(".");
    }

    QString screenshotFileName;
    int screenshotIndex = 0;
    do
    {
        screenshotFileName = screenshotFileDir + QString("/%1.jpg").arg(screenshotIndex++, 4, 10, QChar('0'));
    }
    while (QFileInfo(screenshotFileName).exists());

    auto screenshotFileNameUtf8 = screenshotFileName.toUtf8();
    const char* screenshotCmd[] = {"screenshot_to_file", screenshotFileNameUtf8.constData(), NULL};
    mpv_command(this->mpv, screenshotCmd);

    const char* showTextCmd[] = {"show_text", "Screenshot taken", NULL};
    mpv_command(this->mpv, showTextCmd);
}

void PlayerWindow::tweet()
{
    QSettings settings;
    settings.beginGroup("Twitter");

    QProcess* tweetPy = new QProcess(this);
    tweetPy->start(Hook::path("tweet.py"),
                   QStringList() << this->screenshotsDir()
                                 << settings.value("consumer_key").toString()
                                 << settings.value("consumer_secret").toString()
                                 << settings.value("access_token_key").toString()
                                 << settings.value("access_token_secret").toString());
}
