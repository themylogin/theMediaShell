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

#include "Player/PlaylistModel.h"

class PlayerWindow : public QMainWindow
{
    Q_OBJECT

public:
    PlayerWindow(QString playlistName, QString playlistTitle, QStringList playlist, QWidget* parent = 0);

signals:
    void closing(bool poweringOff);

public slots:
    void showSidebar();
    void showSidebarTemporarily();
    void hideSidebar();

protected:
    bool event(QEvent* event);

private:
    QString playlistName;

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
    bool findDuration(QString stdout, float& duration);

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
    mpv_handle* mpv;
    void initPlayer();

    bool paused;
    double progress;
    double notifiedProgress;
    double duration;
    double remaining;
    void resetPlayerProperties();

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

    // Hooks
    void runHook(const QString& hookName);


private slots:
    // Sidebar
    void updateClockLabel();
    void updatePowerLabel();
    void togglePowerOffOnFinish();
    void toggleSidebar();

    // Player
    void stop();

    void planLess();
    void planMore();

    void openingEndsHere();
    void endingStartsHere();
};

#endif // PLAYERWINDOW_H
