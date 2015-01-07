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

    bool setStyleSheetFromFile(QWidget* widget, QString fileName);
    void resizeMessageBox(QMessageBox* box, int width = 1280);

    void x11KeyEventForChildren(WId win, bool press, quint32 keysum, quint32 modifiers);

    double determineDuration(const QString& path);
}

#endif // UTILS_H
