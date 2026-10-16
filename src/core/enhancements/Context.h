#pragma once

#include "core/enhancements/Lookups.h"
#include "core/enhancements/RecordStore.h"
#include "core/tags/TagBuilder.h"

#include <QHash>
#include <QMap>
#include <QString>

#include <memory>

namespace core::enh {

struct Context
{
    std::shared_ptr<const RecordStore> store;
    const Loc *loc = nullptr;
    const Loc *tagLoc = nullptr;

    ScitemLookups scitem;
    RecordLookup vehicleAmmo;
    RecordLookup fpsAmmo;
    RecordLookup controllers;
    RecordLookup armor;
    QHash<QString, qint64> reputation;
    Standings standings;

    QMap<QString, tags::TagConfig> tagConfigs;
    bool annotateMissionDescs = true;
    QString repXpLabel = QStringLiteral("Rep");
    QHash<QString, QString> missionHeaders;
    QString missionHeaderEm = QStringLiteral("EM3");
    QHash<QString, bool> missionDetailFields;
    QHash<QString, bool> missionTitleTags;
    bool statsPrepend = false;
    bool rsOreNameAnnotations = true;

    const tags::TagConfig &config(const QString &category) const;
    QString missionHeader(const QString &key) const;
    bool showField(const QString &field) const { return missionDetailFields.value(field, true); }
    bool showTitleTag(const QString &field) const { return missionTitleTags.value(field, true); }
};

}
