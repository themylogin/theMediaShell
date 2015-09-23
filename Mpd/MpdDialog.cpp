#include "MpdDialog.h"

#include "Utils.h"

MpdDialog::MpdDialog(QString movie, MpdClient* mpd, bool waitAlsa, QWidget* parent) :
    QMessageBox(parent)
{
    this->setIcon(QMessageBox::Information);

    this->movie = movie;

    this->mpd = mpd;
    this->waitAlsa = waitAlsa;
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
        this->mpd->pause(this->waitAlsa);
        return true;
    }
    else
    {
        return !this->mpdState.playing;
    }
}

bool MpdDialog::waitMusicOver(QString movie, MpdClient* mpd, bool waitAlsa, bool* wasPlaying, QWidget* parent)
{
    if (*wasPlaying)
    {
        MpdDialog dialog(movie, mpd, waitAlsa, parent);
        Utils::setStyleSheetFromFile(&dialog, "://QMessageBox.qss");
        dialog.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
        Utils::resizeMessageBox(&dialog);
        return dialog.canPlay(wasPlaying);
    }
    else
    {
        return true;
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
            this->mpd->pause(this->waitAlsa);
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
