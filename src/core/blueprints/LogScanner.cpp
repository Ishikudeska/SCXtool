#include "core/blueprints/LogScanner.h"

#include "core/text/PyText.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTimeZone>

#include <algorithm>

namespace core::blueprints {

namespace {

constexpr QLatin1StringView kLiveLogName("Game.log");
constexpr QLatin1StringView kLogBackupsDir("logbackups");
constexpr QByteArrayView kCheapMarker = "Received Blueprint:";

const QRegularExpression &eventRe()
{
    static const QRegularExpression re(
        QStringLiteral(R"re(<(\d{4})-(\d{2})-(\d{2})T(\d{2}):(\d{2}):(\d{2})(?:\.(\d+))?Z>)re"
                       R"re(.*?<SHUDEvent_OnNotification> Added notification )re"
                       R"re("Received Blueprint: (.*?): " \[)re"));
    return re;
}

QDateTime timestampOf(const QRegularExpressionMatch &m)
{
    const auto n = [&](int group) { return m.capturedView(group).toInt(); };
    const QDate date(n(1), n(2), n(3));
    int ms = 0;
    if (m.hasCaptured(7))
        ms = (m.captured(7) + QStringLiteral("00")).first(3).toInt();
    const QTime time(n(4), n(5), n(6), ms);
    if (!date.isValid() || !time.isValid())
        return {};
    return QDateTime(date, time, QTimeZone::UTC);
}

QDateTime floorFor(const QDateTime &since)
{
    return since.isValid() ? since : blueprintEpoch();
}

}

QDateTime blueprintEpoch()
{
    return QDateTime(QDate(2026, 3, 1), QTime(0, 0), QTimeZone::UTC);
}

QList<BlueprintEvent> parseEvents(QStringView text)
{
    QList<BlueprintEvent> events;
    for (const QRegularExpressionMatch &m : eventRe().globalMatchView(text)) {
        const QDateTime ts = timestampOf(m);
        const QString name = py::strip(m.capturedView(8));
        if (ts.isValid() && !name.isEmpty())
            events.append({ts, name});
    }
    return events;
}

QStringList findLogFiles(const QString &channelDir, const QDateTime &since)
{
    const qint64 floor = floorFor(since).toMSecsSinceEpoch();

    QFileInfoList candidates;
    const QDir backups(QDir(channelDir).filePath(kLogBackupsDir));
    if (backups.exists())
        candidates =
            backups.entryInfoList({QStringLiteral("*.log")}, QDir::Files | QDir::Hidden | QDir::System);
    const QFileInfo live(QDir(channelDir).filePath(kLiveLogName));
    if (live.isFile())
        candidates.append(live);

    struct Kept
    {
        qint64 mtime;
        QString path;
    };
    QList<Kept> kept;
    for (const QFileInfo &info : std::as_const(candidates)) {
        const QDateTime mtime = info.lastModified(QTimeZone::UTC);
        if (!mtime.isValid() || mtime.toMSecsSinceEpoch() < floor)
            continue;
        kept.append({mtime.toMSecsSinceEpoch(), QDir::toNativeSeparators(info.absoluteFilePath())});
    }
    std::sort(kept.begin(), kept.end(), [](const Kept &a, const Kept &b) {
        return a.mtime != b.mtime ? a.mtime < b.mtime : py::less(a.path, b.path);
    });

    QStringList paths;
    for (const Kept &k : kept)
        paths << QDir::fromNativeSeparators(k.path);
    return paths;
}

ScanResult scanFiles(const QStringList &paths, const QDateTime &since, const ScanProgress &progress)
{
    const QDateTime epoch = blueprintEpoch();
    ScanResult result;
    result.filesScanned = static_cast<int>(paths.size());

    for (qsizetype i = 0; i < paths.size(); ++i) {
        if (progress)
            progress(static_cast<int>(i), result.filesScanned, QFileInfo(paths[i]).fileName());
        QFile file(paths[i]);
        if (!file.open(QIODevice::ReadOnly))
            continue;
        while (!file.atEnd()) {
            const QByteArray line = file.readLine();
            if (!line.contains(kCheapMarker))
                continue;
            for (const auto parts = line.split('\r'); const QByteArray &part : parts) {
                for (const auto events = parseEvents(QString::fromUtf8(part));
                     const BlueprintEvent &ev : events) {
                    if (ev.timestamp < epoch)
                        continue;
                    if (!result.latestTimestamp.isValid() || ev.timestamp > result.latestTimestamp)
                        result.latestTimestamp = ev.timestamp;
                    if (since.isValid() && ev.timestamp <= since)
                        continue;
                    result.names.insert(ev.name);
                    ++result.eventsMatched;
                }
            }
        }
    }

    if (progress)
        progress(result.filesScanned, result.filesScanned, QString());
    return result;
}

ScanResult scanChannel(const QString &channelDir, const QDateTime &since, const ScanProgress &progress)
{
    return scanFiles(findLogFiles(channelDir, since), since, progress);
}

QStringList channelsToScan(const QString &activeChannel, bool includeLinked, const QStringList &installed)
{
    static const QStringList linked = {QStringLiteral("HOTFIX"), QStringLiteral("LIVE")};
    QStringList channels = {activeChannel};
    if (includeLinked && linked.contains(activeChannel)) {
        for (const QString &other : linked)
            if (other != activeChannel && installed.contains(other))
                channels << other;
    }
    return channels;
}

}
