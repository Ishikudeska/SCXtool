#pragma once

#include "core/blueprints/OwnedItems.h"
#include "core/model/StringEntry.h"
#include "core/text/IniFile.h"

#include <QList>
#include <QSet>
#include <QString>
#include <QStringList>

#include <array>
#include <optional>
#include <utility>

namespace core::table {

enum Column {
    ColCategory,
    ColKey,
    ColDefault,
    ColCurrent,
    ColStar,
    ColOrder,
    ColCustom,
    ColStatus,
    ColOwned,
    ColumnCount
};

QString sortOrder(const QString &customValue, const QString &favoritePrefix);

QString withSortOrder(const QString &customValue, const QString &originalValue, const QString &favoritePrefix,
                      const QString &order);

bool isFavorite(const StringEntry &entry, const QString &favoritePrefix);

bool toggleFavorite(StringEntry &entry, const QString &favoritePrefix);

void setCustomValue(StringEntry &entry, const QString &value);

struct FilterCriteria
{
    std::array<QString, ColumnCount> columnText;
    QString searchText;
    QString category = QStringLiteral("All");
    QString status = QStringLiteral("All");
    bool hideUnmodified = false;
    bool favoritesOnly = false;
    bool shipVehicleNamesOnly = false;
    bool bpTitlesOnly = false;
    bool bpDescsOnly = false;
    QString favoritePrefix = QStringLiteral("*");
    std::optional<QString> bpHeader;
};

QList<int> filterEntryIndices(const QList<StringEntry> &entries, const IniMap &defaults,
                              const FilterCriteria &criteria);

std::pair<QString, int> groupSortKey(const QString &key);

struct OwnedState
{
    QSet<QString> blueprintItems;
    QSet<QString> owned;
    blueprints::Enclosings enclosings = blueprints::defaultEnclosings();
};

QString ownedName(const StringEntry &entry, const IniMap &defaults, const OwnedState &owned);
bool isBlueprintItem(const StringEntry &entry, const IniMap &defaults, const OwnedState &owned);
bool isOwned(const StringEntry &entry, const IniMap &defaults, const OwnedState &owned);

void sortIndices(QList<int> &indices, const QList<StringEntry> &entries, const IniMap &defaults,
                 Column column, bool descending, bool grouped, const QString &favoritePrefix,
                 const OwnedState &owned);

QStringList filterCategories(const QList<StringEntry> &entries);

QString filteredAsTsv(const QList<StringEntry> &entries, const QList<int> &rows);

QString journalStampFor(const StringEntry &entry, const QString &appName, const QString &version);

QString previewHtml(const QString &key, const QString &raw, const QString &stamp = {});

QHash<QString, QString> pendingEdits(const QList<StringEntry> &entries);
int restorePendingEdits(QList<StringEntry> &entries, const QHash<QString, QString> &pending);

}
