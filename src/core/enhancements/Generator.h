#pragma once

#include "core/tags/TagBuilder.h"
#include "core/util/Progress.h"

#include <QHash>
#include <QJsonObject>
#include <QMap>
#include <QSet>
#include <QString>
#include <QStringList>

#include <expected>
#include <optional>

namespace core {
class Settings;
}

namespace core::enh {

inline const QStringList kGeneratorCategories = {
    QStringLiteral("ship_descs"),
    QStringLiteral("component_descs"),
    QStringLiteral("ship_weapon_descs"),
    QStringLiteral("fps_weapon_descs"),
    QStringLiteral("mission_rewards"),
    QStringLiteral("commodity_crafting"),
    QStringLiteral("journal"),
    QStringLiteral("missile_enhancements"),
    QStringLiteral("medical_consumables"),
};

struct GeneratorOptions
{
    QString baseIni;
    QString englishBaseIni;
    QString forgeDir;
    std::optional<QSet<QString>> categories;
    QString patchesDir;

    QMap<QString, tags::TagConfig> tagConfigs;
    bool annotateMissionDescs = true;
    QString repXpLabel = QStringLiteral("Rep");
    QHash<QString, QString> missionHeaders;
    QString missionHeaderEmTag = QStringLiteral("EM3");
    QHash<QString, bool> missionDetailFields;
    QHash<QString, bool> missionTitleTags;
    bool statsPrepend = false;
    bool standardizeEarnableShipNames = false;
    bool rsOreNameAnnotations = true;

    int threads = 0;
};

struct GeneratorResult
{
    QStringList files;
    qsizetype entries = 0;
};

QString outputFileName(const QString &category);

GeneratorOptions optionsFromSettings(Settings &settings);

void applyOptionsJson(GeneratorOptions &options, const QJsonObject &json);

std::expected<GeneratorResult, QString> generateEnhancements(const GeneratorOptions &options,
                                                             ProgressSink *progress = nullptr,
                                                             const CancelToken *cancel = nullptr);

}
