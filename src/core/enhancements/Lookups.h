#pragma once

#include "core/enhancements/RecordStore.h"
#include "core/enhancements/Stats.h"
#include "core/enhancements/Tags.h"

#include <QHash>
#include <QString>

#include <optional>

namespace core::enh {

RecordLookup buildAmmoLookup(const RecordStore &store, const QString &relDir);

struct ScitemLookups
{
    MagazineLookup magazines;
    QHash<QString, QString> entityNames;
    QHash<QString, QString> entityNamesByFilename;
    QHash<QString, QString> entityNameTags;
};
ScitemLookups buildScitemLookups(const RecordStore &store, const Loc &loc,
                                 const tags::TagConfig *componentsConfig);

RecordLookup buildControllerLookup(const RecordStore &store);
RecordLookup buildArmorLookup(const RecordStore &store);

QHash<QString, qint64> buildReputationLookup(const RecordStore &store);

struct Standings
{
    QHash<QString, QString> ranks;
    QHash<QString, QString> tracks;
};
Standings buildStandings(const RecordStore &store, const Loc &englishLoc);

}
