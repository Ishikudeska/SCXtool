#pragma once

#include "core/tags/TagBuilder.h"

#include <QHash>
#include <QList>
#include <QMap>
#include <QSet>
#include <QString>
#include <QStringList>

#include <optional>
#include <utility>

namespace core::blueprints {

using EnclosingPair = std::pair<QString, QString>;
using Enclosings = QList<EnclosingPair>;

const Enclosings &defaultEnclosings();
inline const EnclosingPair kNoneStyleEnclosing = {QString(), QString()};

inline const QString kBpSectionHeader = QStringLiteral("POTENTIAL BLUEPRINTS");
inline const QString kAltBpSectionHeader = QStringLiteral("MULTIPLE BLUEPRINT POOLS");

const QHash<QString, QString> &bulletNameAliases();

std::optional<QString> stripViaStockDiff(const QString &tagged, const QString &stock);

std::optional<std::pair<QString, QString>> findNoneStyleTagWord(const QString &s);
QString stripNoneStyleTagHeuristic(const QString &s);

Enclosings enclosingsFromTagConfigs(const QMap<QString, tags::TagConfig> &configs,
                                    const QStringList &categories = {QStringLiteral("components"),
                                                                     QStringLiteral("missiles"),
                                                                     QStringLiteral("ship_weapons")});

bool hasBpSection(const QString &value, const QString &bpHeader = {});

QString normalizeItemName(const QString &name, const Enclosings &enclosings = defaultEnclosings(),
                          const QString &stock = {});

std::optional<QString> resolveAgainstCatalogue(const QString &name, const QSet<QString> &catalogue);

struct OwnedRepair
{
    QSet<QString> repaired;
    QMap<QString, std::optional<QString>> renamed;
};
OwnedRepair repairForeignOwnedNames(const QSet<QString> &owned, const QSet<QString> &catalogue);

QSet<QString> extractBpItemNames(const QString &value, const Enclosings &enclosings = defaultEnclosings(),
                                 const QString &bpHeader = {});

QString applyOwnedToValue(const QString &value, const QSet<QString> &owned,
                          const Enclosings &enclosings = defaultEnclosings(), const QString &bpHeader = {});

}
