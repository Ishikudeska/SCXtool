#pragma once

#include "core/enhancements/Context.h"

#include <QHash>
#include <QMap>
#include <QSet>
#include <QString>
#include <QStringList>

#include <optional>
#include <utility>
#include <vector>

namespace core::enh {

template <class V> class OrderedMap
{
public:
    using Entry = std::pair<QString, V>;

    V &operator[](const QString &key)
    {
        if (const auto it = index_.constFind(key); it != index_.cend())
            return entries_[*it].second;
        index_.insert(key, entries_.size());
        entries_.push_back({key, V{}});
        return entries_.back().second;
    }
    const V *find(const QString &key) const
    {
        const auto it = index_.constFind(key);
        return it == index_.cend() ? nullptr : &entries_[*it].second;
    }
    bool contains(const QString &key) const { return index_.contains(key); }
    qsizetype size() const { return qsizetype(entries_.size()); }
    bool isEmpty() const { return entries_.empty(); }
    auto begin() { return entries_.begin(); }
    auto end() { return entries_.end(); }
    auto begin() const { return entries_.cbegin(); }
    auto end() const { return entries_.cend(); }

private:
    std::vector<Entry> entries_;
    QHash<QString, std::size_t> index_;
};

QString classifyMissionEngagement(const QString &locKey);
QString routeTokenRole(const QString &var);
bool isRouteTitle(const QString &titleKey);
bool titleHasRouteToken(const QString &title);
QString titleRouteToken(const QString &var, const QString &bodyToken, const QString &locationDetail);

using RouteTokens = OrderedMap<QString>;
using RouteExpandCache = QHash<QString, std::pair<RouteTokens, RouteTokens>>;
std::pair<RouteTokens, RouteTokens> expandNestedRouteVars(const QString &var, const Loc &loc,
                                                          RouteExpandCache *cache);
QString deriveRouteFragment(const QList<const QString *> &bodies, const tags::TagConfig *config,
                            const Loc &loc, RouteExpandCache *cache);
Loc sizeAbbreviationOverrides(const Loc &loc, const QSet<QString> &shortenedSizes);

enum class Spawn { Hostile, Friendly, Objective, Unknown };
struct SpawnBreakdown
{
    QMap<QString, qint64> buckets[4];
    QMap<QString, qint64> &operator[](Spawn s) { return buckets[int(s)]; }
    const QMap<QString, qint64> &operator[](Spawn s) const { return buckets[int(s)]; }
    bool any() const;
};

std::pair<Spawn, QString> classifySpawnGroup(const QString &name, const QString &kind);
SpawnBreakdown extractSpawnCounts(Node element, const QStringList &excludeWithin = {});
QStringList formatSpawnLines(const SpawnBreakdown &breakdown);
void mergeSpawnBreakdownsMax(SpawnBreakdown &into, const SpawnBreakdown &src);
QString extractTurretInfo(Node root);

int parseDifficultyRating(const QString &value);
QString extractDifficulty(Node element);
QString repRewardLine(const QString &fieldName, const QString &amount, const QString &repXpLabel,
                      const QString &track = {});
QStringList extractMissionFlags(Node root);
qint64 extractMissionXp(Node root, const QHash<QString, qint64> &reputation);
QString enhancementsMission(Node root, const QHash<QString, qint64> &reputation, const QString &repXpLabel,
                            const Context &ctx, const QSet<QString> *spawnAmbiguousKeys);

QString stripCigSizePrefix(const QString &name);
QString poolRankLabel(const QString &poolName);
QString nameFromBlueprintFilename(const QString &path);

struct BlueprintPools
{
    QHash<QString, QStringList> items;
    QHash<QString, QString> names;
};
BlueprintPools buildBlueprintPoolLookup(const RecordStore &store, const QHash<QString, QString> &entityNames,
                                        const QHash<QString, QString> &entityNamesByFilename,
                                        const QHash<QString, QString> &entityNameTags,
                                        const QString &placement,
                                        const QHash<QString, QString> &nameFallbackTags);

using FingerprintMap = std::vector<std::pair<QStringList, std::vector<std::pair<QString, QString>>>>;
QStringList buildBlueprintBodyParts(const FingerprintMap &uniqueFps, bool allowOverrides);

std::vector<int> rsValueSteps(const QString &ore);
QStringList formatRsDetailsLines(const QStringList &ores, const Loc &loc);
Loc mineableRsNameOverrides(const Loc &loc);
QString formatRsTag(const QStringList &ores);
QStringList battagliaContractOres(Node contract);

Loc generateMissions(const Context &ctx);

}
