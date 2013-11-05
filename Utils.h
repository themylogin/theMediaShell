#ifndef UTILS_H
#define UTILS_H

#include <QDir>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QWidget>

namespace Utils
{
    bool isMovie(QString path);

    QString formatDuration(int duration);
    QStringList listSubdirectories(QDir directory);
    QProcess* runMplayer(const QString& file, const QStringList& userArguments = QStringList());

    bool setStyleSheetFromFile(QWidget* widget, QString fileName);
}

#endif // UTILS_H
