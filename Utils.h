#ifndef UTILS_H
#define UTILS_H

#include <QDir>
#include <QMessageBox>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QWidget>

namespace Utils
{
    bool isMovie(QString path);

    bool isWatched(float progress, float duration);

    QString formatDuration(int duration);
    QStringList listSubdirectories(QDir directory);
    QProcess* runMplayer(const QString& file, const QStringList& userArguments = QStringList());

    bool setStyleSheetFromFile(QWidget* widget, QString fileName);
    void resizeMessageBox(QMessageBox* box, int width = 1280);

    void x11KeyEventForChildren(WId win, bool press, quint32 keysum, quint32 modifiers);

    bool getFrameExtents(WId win, int& left, int& right, int& top, int& bottom);
}

#endif // UTILS_H
