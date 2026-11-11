#include "core/gamesettings/GameProfile.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>

#ifdef Q_OS_WIN
#include <windows.h>
#include <tlhelp32.h>
#endif

namespace core::gamesettings {

namespace {

constexpr QLatin1StringView kGameExe("StarCitizen.exe");

}

QString resolveCaseInsensitive(const QString &base, const QStringList &parts)
{
    QString current = base;
    bool exists = QFileInfo(current).isDir();
    for (const QString &part : parts) {
        QString next = QDir(current).filePath(part);
        const QStringList entries =
            exists ? QDir(current).entryList(QDir::AllEntries | QDir::NoDotAndDotDot) : QStringList();
        if (!entries.contains(part)) {
            for (const QString &entry : entries) {
                if (entry.compare(part, Qt::CaseInsensitive) == 0) {
                    next = QDir(current).filePath(entry);
                    break;
                }
            }
        }
        current = next;
        exists = exists && QFileInfo::exists(current);
    }
    return current;
}

ProfilePaths profilePaths(const QString &channelInstallDir)
{
    if (channelInstallDir.isEmpty())
        return {};
    ProfilePaths paths;
    const QString client = resolveCaseInsensitive(
        channelInstallDir, {QStringLiteral("user"), QStringLiteral("client"), QStringLiteral("0")});
    paths.dir = resolveCaseInsensitive(client, {QStringLiteral("Profiles"), QStringLiteral("default")});
    paths.attributes = resolveCaseInsensitive(paths.dir, {QStringLiteral("attributes.xml")});
    paths.actionMaps = resolveCaseInsensitive(paths.dir, {QStringLiteral("actionmaps.xml")});
    paths.mappingsDir =
        resolveCaseInsensitive(client, {QStringLiteral("controls"), QStringLiteral("mappings")});
    return paths;
}

bool isGameRunning()
{
#ifdef Q_OS_WIN
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
        return false;
    PROCESSENTRY32W entry{};
    entry.dwSize = sizeof(entry);
    bool found = false;
    for (BOOL ok = Process32FirstW(snapshot, &entry); ok && !found; ok = Process32NextW(snapshot, &entry))
        found = QString::fromWCharArray(entry.szExeFile).compare(kGameExe, Qt::CaseInsensitive) == 0;
    CloseHandle(snapshot);
    return found;
#else
    QDirIterator it(QStringLiteral("/proc"), QDir::Dirs | QDir::NoDotAndDotDot);
    while (it.hasNext()) {
        const QString dir = it.next();
        bool numeric = false;
        QFileInfo(dir).fileName().toLongLong(&numeric);
        if (!numeric)
            continue;
        QFile cmdline(dir + QStringLiteral("/cmdline"));
        if (!cmdline.open(QIODevice::ReadOnly))
            continue;
        const QByteArray args = cmdline.read(4096);
        const QList<QByteArray> parts = args.split('\0');
        if (parts.isEmpty())
            continue;
        const QString program = QString::fromUtf8(parts.front()).replace(u'\\', u'/');
        if (program.section(u'/', -1).compare(kGameExe, Qt::CaseInsensitive) == 0)
            return true;
    }
    return false;
#endif
}

}
