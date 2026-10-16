#pragma once

#include "core/blueprints/BlueprintMeta.h"
#include "core/blueprints/OwnedItems.h"

#include <QDateTime>
#include <QMap>
#include <QSet>
#include <QString>

#include <expected>

namespace core::blueprints {

QString exportOwnedBlueprintsJson(const QSet<QString> &owned, const QMap<QString, BlueprintItem> &meta,
                                  const QDateTime &now = QDateTime::currentDateTimeUtc());
QString exportOwnedBlueprintsCsv(const QSet<QString> &owned, const QMap<QString, BlueprintItem> &meta);

std::expected<QSet<QString>, QString> parseImportNames(const QString &path,
                                                       const Enclosings &enclosings = defaultEnclosings());

struct ImportMatch
{
    QSet<QString> matched;
    QSet<QString> unmatched;
};
ImportMatch matchImportNames(const QSet<QString> &imported, const QSet<QString> &known,
                             const QSet<QString> &catalogue = {},
                             const Enclosings &enclosings = defaultEnclosings());

}
