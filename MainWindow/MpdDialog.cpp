#include "MpdDialog.h"

MpdDialog::MpdDialog(QString movie, MpdClient* mpd, QWidget* parent) :
    QMessageBox(parent)
{
    this->setIcon(QMessageBox::Information);
    this->setText(QString::fromUtf8("Wanted to watch %1?").arg(movie));

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
        this->setInformativeText(QString::fromUtf8("But %1 — %2 is playing! Remaining time: %3:%4").arg(state.artist)
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
