#ifndef PLAYERWINDOW_H
#define PLAYERWINDOW_H

#include <functional>

#include <QLabel>
#include <QMap>
#include <QMainWindow>
#include <QHBoxLayout>
#include <QTableView>
#include <QTableWidget>
#include <QTimer>
#include <QVBoxLayout>

#include <QtNetwork>

#include "mpv/client.h"

#include "Mpd/MpdClient.h"
#include "Player/PlaylistModel.h"

class PlayerWindow : public QMainWindow
{
    Q_OBJECT

public:
    PlayerWindow(QString playlistName, QString playlistTitle, QStringList playlist,
                 MpdClient* mpd, bool* mpdWasPlaying,
                 QWidget* parent = 0);
    ~PlayerWindow();

signals:
    void closing(bool powerOff);

public slots:
    void showSidebar();
    void showSidebarTemporarily();
    void hideSidebar();

protected:
    bool event(QEvent* event);
    bool nativeEvent(const QByteArray& eventType, void* message, long* result);

private:
    QString playlistName;
    QString playlistTitle;

    // Sidebar
    QWidget* sidebar;
    QVBoxLayout* sidebarLayout;
    QTimer showSidebarTemporarilyTimer;
    void initSidebar(const QString& playlistTitle, const QStringList& playlist);

    QHBoxLayout* clockLayout;
    QLabel* clockLabel;
    QTimer* clockTimer;
    QLabel* powerLabel;
    bool powerOffOnFinish;
    void initClock();

    QString title;
    QLabel* titleLabel;
    void initTitle(const QString& playlistTitle);

    PlaylistModel* playlist;
    QTableView* playlistView;
    void initPlaylist(const QStringList& playlist);

    void determineDurations(QStringList playlist);

    QLabel* helpLabel;
    QTableWidget* helpTable;
    QMap<int, std::function<void ()>> hotkeys;
    void initHelp();

    double openingLength;
    double endingLength;
    void initOpeningEnding();
    void drawOpeningEndingLength();

    // Player
    QWidget* mpvContainer;
    QWidget* mpvContainerPseudoFocusDistracter;
    QTimer* mpvContainerPseudoFocusDistracterTimer;
    mpv_handle* mpv;
    bool upscale;
    void initPlayer();
    void resizeMpvContainer();
    void toggleUpscale();

    bool paused;
    double progress;
    double notifiedProgress;
    double duration;
    double remaining;
    int64_t width;
    int64_t height;
    void resetPlayerProperties();

    QString file;
    void play();
    void createMpv(const QString& file);
    void tuneMpv(const QString& file);
    void handleMpvEvent(mpv_event* event);

    // themylog
    QUdpSocket* themylogSocket;
    QHostAddress themylogAddress;
    quint16 themylogPort;
    void initThemylog();

    void themylog(const QString& logger, const QString& level, const QString& msg, const QString& movie, const QMap<QString, QString>& args);
    void themylog(const QString& msg, const QString& movie, const QMap<QString, QString>& args);

    QMap<QString, QString> withProgresDuration(QMap<QString, QString> args);
    QMap<QString, QString> withDownloadedAt(QMap<QString, QString> args, QString file);

    // MPD
    MpdClient* mpd;
    bool* mpdWasPlaying;

    QString screenshotsDir();


private slots:
    // Sidebar
    void updateClockLabel();
    void updatePowerLabel();
    void togglePowerOffOnFinish();
    void toggleSidebar();

    // Player
    void distractFocusFromMpvContainer();

    void togglePause();
    void stop();

    void planLess();
    void planMore();

    void openingEndsHere();
    void endingStartsHere();

    void takeScreenshot();
    void tweet();
};

#endif // PLAYERWINDOW_H
