#ifndef MPDDIALOG_H
#define MPDDIALOG_H

#include <QMessageBox>
#include <QPushButton>

#include "MpdClient.h"

class MpdDialog : public QMessageBox
{
    Q_OBJECT
public:
    explicit MpdDialog(QString movie, MpdClient* mpd, QWidget* parent = 0);
    bool canPlay(bool* wasPlaying);

signals:

public slots:

private slots:
    void mpdStateChanged(MpdState state);

private:
    MpdClient* mpd;
    MpdState mpdState;
    bool wasPlaying;
    bool wasPausedByUs;

    QPushButton* pauseButton;
    QPushButton* escapeButton;
};

#endif // MPDDIALOG_H
