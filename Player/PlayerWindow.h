#ifndef PLAYERWINDOW_H
#define PLAYERWINDOW_H

#include <QLabel>
#include <QHBoxLayout>
#include <QProcess>
#include <QTableView>
#include <QTableWidget>
#include <QTimer>
#include <QVBoxLayout>

#include <QtNetwork>

#include <QxtGlobalShortcut>

#include "mpv/client.h"

#include "Player/PlaylistModel.h"

class PlayerWindow : public QWidget
{
    Q_OBJECT

public:
    PlayerWindow(QString playlistName, QString playlistTitle, QStringList playlist, QWidget* parent = 0);

public slots:
    void hide();
    void show();
    void showTemporarily();
    bool isVisible() const;

protected:
    void closeEvent(QCloseEvent* event);

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

    QUdpSocket* themylogSocket;
    QHostAddress themylogAddress;
    quint16 themylogPort;
    QDateTime themylogLastNotify;

    bool findDuration(QString stdout, float& duration);
    void determineDurations(QStringList playlist);

    void play();

    QString getHookPath(QString hookName);

    void themylog(QString string);

private slots:
    void updateClockLabel();
    void updatePowerLabel();
    void togglePowerOffOnFinish();

    void checkEvents();

    void drawOpeningEndingLength();

    void stop();

    void toggle();

    void planLess();
    void planMore();

    void openingEndsHere();

    void endingStartsHere();
};

#endif // PLAYERWINDOW_H
