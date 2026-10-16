#pragma once

#include "core/blueprints/OwnedItems.h"
#include "core/model/StringEntry.h"

#include <QHash>
#include <QList>
#include <QMap>
#include <QSet>
#include <QString>

#include <span>
#include <utility>

namespace core::blueprints {

struct BlueprintItem
{
    QString name;
    QSet<QString> missions;
    QString type;
    QString cls;
    QString size;
    QString grade;
    QString taggedName;

    bool operator==(const BlueprintItem &) const = default;
};

std::span<const std::pair<const char *, const char *>> manualBlueprintItems();
inline const QString kManualMissionLabel = QStringLiteral("Limited time event reward");

QString expandClassFullWord(const QString &cls);
QString stripSizePrefix(const QString &size);

std::pair<QString, bool> stripRawBlueprintFilenamePrefix(const QString &stem);

struct ComponentTag
{
    QString cls;
    QString size;
    QString grade;
    bool operator==(const ComponentTag &) const = default;
};
ComponentTag parseComponentTag(const QString &value, const Enclosings &enclosings = defaultEnclosings(),
                               const QString &stock = {});

QString sizeFromKey(const QString &key);
QString componentTypeFromKey(const QString &key);
QString blueprintTypeFromKey(const QString &key);
QString cleanMissionTitle(const QString &value);

QSet<QString> knownItemNames(const QList<StringEntry> &entries,
                             const Enclosings &enclosings = defaultEnclosings(),
                             const QHash<QString, QString> &defaultValues = {});

QMap<QString, BlueprintItem> buildBlueprintMetadata(const QList<StringEntry> &entries,
                                                    const Enclosings &enclosings = defaultEnclosings(),
                                                    const QHash<QString, QString> &defaultValues = {},
                                                    const QString &bpHeader = {});

}
