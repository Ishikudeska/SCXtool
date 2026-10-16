#pragma once

#include "core/enhancements/Common.h"
#include "core/tags/TagBuilder.h"

#include <QHash>
#include <QString>

#include <functional>
#include <optional>
#include <vector>

namespace core::enh {

struct RecordLookup
{
    QHash<QString, Node> byId;
    std::vector<XmlDoc> docs;
    Node value(const QString &id) const { return byId.value(id); }
};

using NameTagger = std::function<QString(const QString &desc, Node root)>;

QString componentNameTag(const QString &desc, Node root, const tags::TagConfig *config = nullptr,
                         const QString &componentType = {});
QString missileNameTag(const QString &desc, Node root, const tags::TagConfig *config = nullptr);

std::optional<tags::TagConfig> miningLaserTagConfig(const tags::TagConfig *componentsConfig);
QString miningLaserComponentTag(const QString &desc, Node root,
                                const std::optional<tags::TagConfig> &miningConfig);

NameTagger shipWeaponNameTagger(const RecordLookup &ammo, const tags::TagConfig *config,
                                const tags::TagConfig *miningLaserConfig);

const tags::ElementSpec *componentElement(const tags::TagConfig &config, const QString &kind);

QString bareTypeTagFromDesc(const QString &desc, const tags::TagConfig *componentsConfig);
Loc bareTypeTags(const Loc &loc, const tags::TagConfig *componentsConfig);
QHash<QString, QString> bareTypeNameTagLookup(const Loc &loc, const tags::TagConfig *componentsConfig);

const tags::TagConfig &defaultTagConfig(const QString &category);

}
