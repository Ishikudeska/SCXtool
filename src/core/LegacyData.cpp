#include "core/LegacyData.h"

#include "core/AppIdentity.h"
#include "core/Settings.h"

#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

namespace core {

namespace {

QString legacyName()
{
    return QString::fromUtf8(identity::kLegacyAppName);
}

QString currentName()
{
    return QString::fromUtf8(identity::kAppName);
}

int mergeInto(const QString &from, const QString &to, QStringList &log)
{
    int kept = 0;
    const QFileInfoList entries =
        QDir(from).entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);
    for (const QFileInfo &entry : entries) {
        const QString target = QDir(to).filePath(entry.fileName());
        const QFileInfo existing(target);
        if (!existing.exists()) {
            if (!QDir().rename(entry.absoluteFilePath(), target)) {
                log << QStringLiteral("Could not move %1 to %2").arg(entry.absoluteFilePath(), target);
                ++kept;
            }
        } else if (entry.isDir() && existing.isDir() && !entry.isSymLink()) {
            kept += mergeInto(entry.absoluteFilePath(), target, log);
        } else {
            ++kept;
        }
    }
    QDir().rmdir(from);
    return kept;
}

void moveDir(const QString &from, const QString &to, QStringList &log)
{
    if (from.isEmpty() || to.isEmpty() || !QFileInfo(from).isDir())
        return;
    if (!QFileInfo::exists(to)) {
        QDir().mkpath(QFileInfo(to).absolutePath());
        if (QDir().rename(from, to))
            log << QStringLiteral("Moved %1 to %2").arg(from, to);
        else
            log << QStringLiteral("Could not move %1 to %2").arg(from, to);
        return;
    }
    if (!QFileInfo(to).isDir())
        return;
    if (const int kept = mergeInto(from, to, log); kept == 0)
        log << QStringLiteral("Moved %1 into %2").arg(from, to);
    else
        log << QStringLiteral("Moved %1 into %2; %3 entries it already had were left in the old folder")
                   .arg(from, to)
                   .arg(kept);
}

QString renamedPath(const QString &path)
{
    const QFileInfo info(QDir::cleanPath(QDir::fromNativeSeparators(path)));
    if (path.isEmpty() || info.fileName() != legacyName())
        return {};
    return QDir(info.path()).filePath(currentName());
}

}

LegacyDataLocations legacyDataLocations()
{
    LegacyDataLocations where;
    where.roots = PathRoots::defaults();
    where.settingsFile = Settings::defaultFilePath();
    where.settingsDirTo = QFileInfo(where.settingsFile).path();
#ifdef Q_OS_WIN
    if (const QString roaming = qEnvironmentVariable("APPDATA"); !roaming.isEmpty())
        where.settingsDirFrom = QDir(QDir::fromNativeSeparators(roaming)).filePath(legacyName());
#else
    where.settingsDirFrom = QDir(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation))
                                .filePath(legacyName() + u'/' + legacyName());
#endif
    return where;
}

QStringList migrateLegacyData(const LegacyDataLocations &where)
{
    QStringList log;
    if (where.roots.portable || legacyName() == currentName())
        return log;

    moveDir(where.settingsDirFrom, where.settingsDirTo, log);
    if (const QString parent = QFileInfo(where.settingsDirFrom).path();
        QFileInfo(parent).fileName() == legacyName())
        QDir().rmdir(parent);

    for (const QString &root : {where.roots.documentsDir, where.roots.localAppData}) {
        if (!root.isEmpty())
            moveDir(QDir(root).filePath(legacyName()), QDir(root).filePath(currentName()), log);
    }

    if (where.settingsFile.isEmpty() || !QFileInfo::exists(where.settingsFile))
        return log;
    Settings settings(where.settingsFile);
    const auto repoint = [&](const QString &value, void (Settings::*set)(const QString &)) {
        const QString to = renamedPath(value);
        if (to.isEmpty())
            return;
        moveDir(QDir::cleanPath(QDir::fromNativeSeparators(value)), to, log);
        if (QFileInfo(to).isDir())
            (settings.*set)(to);
    };
    repoint(settings.userDataDirOverride(), &Settings::setUserDataDirOverride);
    repoint(settings.cacheDirOverride(), &Settings::setCacheDirOverride);
    settings.sync();
    return log;
}

}
