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

bool moveDir(const QString &from, const QString &to, QStringList &log)
{
    if (from.isEmpty() || to.isEmpty() || !QFileInfo(from).isDir() || QFileInfo::exists(to))
        return false;
    QDir().mkpath(QFileInfo(to).absolutePath());
    if (!QDir().rename(from, to)) {
        log << QStringLiteral("Could not move %1 to %2").arg(from, to);
        return false;
    }
    log << QStringLiteral("Moved %1 to %2").arg(from, to);
    return true;
}

// "<parent>/<old name>" becomes "<parent>/<new name>"; anything else, empty.
QString renamedPath(const QString &path)
{
    const QFileInfo info(QDir::cleanPath(QDir::fromNativeSeparators(path)));
    if (path.isEmpty() || info.fileName() != legacyName())
        return {};
    return QDir(info.path()).filePath(currentName());
}

} // namespace

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
    // QSettings' AppConfigLocation: <config>/<org>/<app>, both the app name.
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
    // On Linux the old folder sat inside an <old> organization folder.
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

} // namespace core
