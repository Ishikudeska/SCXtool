#pragma once

#include <QDateTime>
#include <QList>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QStringView>

#include <functional>

namespace core::blueprints {

QDateTime blueprintEpoch();

struct BlueprintEvent
{
    QDateTime timestamp;
    QString name;
};

QList<BlueprintEvent> parseEvents(QStringView text);

QStringList findLogFiles(const QString &channelDir, const QDateTime &since = {});

struct ScanResult
{
    QSet<QString> names;
    QDateTime latestTimestamp;
    int eventsMatched = 0;
    int filesScanned = 0;
};

using ScanProgress = std::function<void(int, int, const QString &)>;

ScanResult scanFiles(const QStringList &paths, const QDateTime &since = {},
                     const ScanProgress &progress = {});
ScanResult scanChannel(const QString &channelDir, const QDateTime &since = {},
                       const ScanProgress &progress = {});

QStringList channelsToScan(const QString &activeChannel, bool includeLinked, const QStringList &installed);

}
