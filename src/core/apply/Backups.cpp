#include "core/apply/Backups.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>

namespace core {

namespace {

QFileInfoList byAge(const QString &dir, const QString &prefix)
{
    return QDir(dir).entryInfoList({prefix + u'*'}, QDir::Files, QDir::Time | QDir::Reversed);
}

}

QString GameFileBackups::backup(const QString &gameFile, QString *error, const QDateTime &now) const
{
    if (!QFileInfo::exists(gameFile))
        return {};
    if (!QDir().mkpath(dir_)) {
        if (error)
            *error = QStringLiteral("Cannot create %1").arg(dir_);
        return {};
    }

    QFileInfoList existing = byAge(dir_, prefix_);
    while (existing.size() >= kKeep) {
        QFile::remove(existing.takeFirst().absoluteFilePath());
    }

    const QString target = QDir(dir_).filePath(prefix_ + now.toString(QStringLiteral("yyyyMMdd_HHmmss")));
    QFile::remove(target);
    if (!QFile::copy(gameFile, target)) {
        if (error)
            *error = QStringLiteral("Cannot back up %1 to %2").arg(gameFile, target);
        return {};
    }
    QFile copy(target);
    if (copy.open(QIODevice::ReadWrite)) {
        copy.setFileTime(QFileInfo(gameFile).lastModified(), QFileDevice::FileModificationTime);
        copy.close();
    }
    return target;
}

QFileInfoList GameFileBackups::list() const
{
    return QDir(dir_).entryInfoList({prefix_ + u'*'}, QDir::Files, QDir::Time);
}

QString GameFileBackups::restore(const QString &backupFile, const QString &gameFile)
{
    QDir().mkpath(QFileInfo(gameFile).absolutePath());
    if (QFileInfo::exists(gameFile) && !QFile::remove(gameFile))
        return QStringLiteral("Cannot replace %1").arg(gameFile);
    if (!QFile::copy(backupFile, gameFile))
        return QStringLiteral("Cannot copy %1 to %2").arg(backupFile, gameFile);
    return {};
}

}
