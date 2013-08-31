#ifndef RENAMEABANDONEDSUBTITLESDIALOG_H
#define RENAMEABANDONEDSUBTITLESDIALOG_H

#include <QDialog>
#include <QListView>
#include <QStringListModel>

#include "MediaModel/MediaModel.h"

class RenameAbandonedSubtitlesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RenameAbandonedSubtitlesDialog(MediaModel* moviesModel, QModelIndex index, QWidget *parent = 0);
    
private:
    QStringListModel* moviesModel;
    QStringListModel* subtitlesModel;

    QListView* moviesView;
    QListView* subtitlesView;
};

#endif // RENAMEABANDONEDSUBTITLESDIALOG_H
