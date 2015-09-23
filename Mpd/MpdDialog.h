#ifndef MPDDIALOG_H
#define MPDDIALOG_H

#include <QMessageBox>
#include <QPushButton>
#include <QString>

#include "MpdClient.h"

class MpdDialog : public QMessageBox
{
    Q_OBJECT
public:
    explicit MpdDialog(QString movie, MpdClient* mpd, bool waitAlsa, QWidget* parent = 0);
    bool canPlay(bool* wasPlaying);

    static bool waitMusicOver(QString movie, MpdClient* mpd, bool waitAlsa, bool* wasPlaying, QWidget* parent = 0);

signals:

public slots:

private slots:
    void mpdStateChanged(MpdState state);

private:
    QString movie;
    MpdClient* mpd;
    bool waitAlsa;
    MpdState mpdState;
    bool wasPlaying;
    bool wasPausedByUs;

    QPushButton* pauseButton;
    QPushButton* escapeButton;
};

#endif // MPDDIALOG_H
