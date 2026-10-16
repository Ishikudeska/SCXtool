#pragma once

#include "core/enhancements/Context.h"

#include <QHash>
#include <QString>
#include <QStringList>

#include <utility>
#include <vector>

namespace core::enh {

QString normalizeCommodityName(const QString &raw);
QString humanizeCraftCategory(const QString &category);
QString craftUsageKey(const QString &categoryPath);
QString craftUsageLegend(const tags::TagConfig *config);
QString qdSizeRange(std::vector<int> sizes);
QStringList condenseCraftedItems(const std::vector<std::pair<QString, QString>> &items);
QHash<QString, QStringList> parseCompendiumLocations(const QString &content);
const QStringList *lookupCommodityLocations(const QHash<QString, QStringList> &locations,
                                            const QString &display, const QString &internalName);
QString commodityTag(const tags::TagConfig *config, bool crafting, bool collection,
                     const QStringList &usageKeys = {});

std::pair<Loc, Loc> generateCommodityJournal(const Context &ctx);

}
