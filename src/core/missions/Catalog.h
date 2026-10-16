#pragma once

#include "core/missions/Places.h"
#include "core/text/IniFile.h"

#include <QString>
#include <QStringList>

#include <atomic>
#include <vector>

namespace core::missions {

struct Payout
{
    enum class Kind { None, Fixed, Calculated };
    Kind kind = Kind::None;
    qint64 amount = 0;
    qint64 max = 0;
    QString currency;
    qint64 buyIn = 0;
};

struct ReputationReward
{
    QString faction;
    QString scope;
    qint64 amount = 0;
    bool success = true;
    bool operator==(const ReputationReward &) const = default;
};

struct BlueprintReward
{
    QString label;
    double chance = 1;
    QStringList items;
};

struct LocationSlot
{
    QString variable;
    QString token;
    int placeSet = -1;
    QStringList searchTags;
};

struct Mission
{
    QString id;
    QString file;
    bool contract = false;
    QString title, description, giver, category, difficulty;
    int titleVariants = 1, descriptionVariants = 1;
    QStringList systems;
    Payout payout;
    std::vector<ReputationReward> reputation;
    std::vector<BlueprintReward> blueprints;
    QString requiredRank;
    std::vector<LocationSlot> locations;
};

struct Catalog
{
    std::vector<Mission> missions;
    std::vector<Place> places;
    std::vector<std::vector<int>> placeSets;
    QStringList categories, systems;
    bool hasPlaces = false;
};

struct CatalogSources
{
    QString recordsDir;
    QString tagTable;
    const IniMap *loc = nullptr;
};

Catalog buildCatalog(const CatalogSources &sources, const std::atomic<bool> *cancel = nullptr);

struct Filter
{
    enum class PayoutKind { Any, Fixed, Calculated };
    enum class Sort { Title, PayoutHigh, PayoutLow };
    QString search;
    QString category;
    QString system;
    PayoutKind payout = PayoutKind::Any;
    bool blueprintsOnly = false;
    Sort sort = Sort::Title;
};

std::vector<int> filterMissions(const Catalog &catalog, const Filter &filter);

}
