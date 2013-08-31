#include "RenameAbandonedSubtitlesDialog.h"

#include <QHBoxLayout>

RenameAbandonedSubtitlesDialog::RenameAbandonedSubtitlesDialog(MediaModel* moviesModel, QModelIndex index, QWidget *parent) :
    QDialog(parent)
{
    QStringList subtitles;
    //VideoIdentification id;
    //if (moviesModel->identification(index, id))
    //{
//        subtitles << id.abandonedSubtitles;
  //  }
    this->subtitlesModel = new QStringListModel(subtitles);

    this->moviesView = new QListView(this);
    this->moviesView->setModel(moviesModel);
    this->moviesView->setRootIndex(index.parent());

    this->subtitlesView = new QListView(this);
    this->subtitlesView->setModel(this->subtitlesModel);

    QHBoxLayout* layout = new QHBoxLayout;
    layout->addWidget(this->moviesView);
    layout->addWidget(this->subtitlesView);
    this->setLayout(layout);
}
