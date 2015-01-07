#include "MpdDialog.h"

#include "Utils.h"

MpdDialog::MpdDialog(QString movie, MpdClient* mpd, QWidget* parent) :
    QMessageBox(parent)
{
    this->setIcon(QMessageBox::Information);

    this->movie = movie;

    this->mpd = mpd;
    connect(this->mpd, SIGNAL(stateChanged(MpdState)), this, SLOT(mpdStateChanged(MpdState)));

    this->pauseButton = this->addButton("Pause it", QMessageBox::ActionRole);
    this->escapeButton = this->addButton("Cancel", QMessageBox::RejectRole);

    this->wasPlaying = false;
    this->wasPausedByUs = false;
    this->mpdState = this->mpd->getState();
    this->mpdStateChanged(this->mpdState);
}

bool MpdDialog::canPlay(bool* wasPlaying)
{
    this->exec();

    if (wasPlaying)
    {
        *wasPlaying = this->wasPlaying || this->wasPausedByUs;
    }

    if (this->clickedButton() == this->pauseButton)
    {
        this->mpd->pause();
        return true;
    }
    else
    {
        return !this->mpdState.playing;
    }
}

void MpdDialog::mpdStateChanged(MpdState state)
{
    if (state.playing)
    {
        this->setText(QString::fromUtf8("Wanted to watch %1?\nBut %2 — %3 is playing!\nRemaining time: %4:%5")
            .arg(this->movie)
            .arg(state.artist)
            .arg(state.title)
            .arg(QString::number(state.remaining / 60), 2, QChar('0'))
            .arg(QString::number(state.remaining % 60), 2, QChar('0')));
        if (this->mpdState.remaining < state.remaining ||
            this->mpdState.artist != state.artist ||
            this->mpdState.title != state.title)
        {
            this->wasPausedByUs = true;
            this->mpd->pause();
        }

        this->mpdState = state;
        this->wasPlaying = true;
    }
    else
    {
        this->mpdState = state;
        this->wasPlaying = false;
        this->escapeButton->click();
    }
}
