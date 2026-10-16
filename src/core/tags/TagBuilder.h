#pragma once

#include <QHash>
#include <QJsonObject>
#include <QList>
#include <QMap>
#include <QSet>
#include <QString>
#include <QStringList>

#include <array>
#include <optional>
#include <span>

namespace core::tags {

inline const QStringList kCategories = {QStringLiteral("components"), QStringLiteral("missiles"),
                                        QStringLiteral("ship_weapons"), QStringLiteral("commodities"),
                                        QStringLiteral("mission_titles")};

inline constexpr QChar kUsageInputSep = QChar(0x1F);

QStringList elementKinds(const QString &category);

struct Option
{
    const char *key;
    const char *label;
};
std::span<const Option> stylesFor(const QString &kind);
QString elementLabel(const QString &kind);

struct TextOption
{
    const char *key;
    const char *label;
    const char *text;
};
std::span<const TextOption> separators();
std::span<const TextOption> routeArrows();
std::span<const TextOption> titleSeparators();
struct Enclosing
{
    const char *key;
    const char *label;
    const char *open;
    const char *close;
};
std::span<const Enclosing> enclosings();
std::span<const Option> placements();
std::span<const Option> missionTitlePlacements();
std::span<const Option> locationDetails();

struct PhraseOption
{
    const char *key;
    const char *label;
    const char *phrase;
    const char *replacement;
};
std::span<const PhraseOption> shortenPhraseOptions();
std::span<const PhraseOption> removeWordOptions();
std::span<const PhraseOption> underlineOptions();

std::span<const std::pair<const char *, const char *>> sizeAbbreviations();

struct UsageCategory
{
    const char *name;
    const char *shortCode;
    const char *medCode;
    const char *longCode;
    const char *group;
};
std::span<const UsageCategory> craftUsageCategories();

QString damageMappingKey(const QString &compactLabel);

using Variants = std::array<QString, 3>;
using Mapping = QMap<QString, Variants>;

const QHash<QString, Mapping> &defaultKindMappings();
bool isMappedKind(const QString &kind);

struct ElementSpec
{
    QString kind;
    bool enabled = true;
    QString style;
    bool operator==(const ElementSpec &) const = default;
};

struct TagConfig
{
    QList<ElementSpec> elements;
    QString separator = QStringLiteral("hyphen");
    QString enclosing = QStringLiteral("square");
    QString placement = QStringLiteral("prepend");
    Mapping classMapping;
    QString usageSeparator = QStringLiteral("pipe");
    QString routeArrow = QStringLiteral("gt");
    QString titleSeparator = QStringLiteral("dash");
    QString locationDetail = QStringLiteral("address");
    QSet<QString> abbreviatedPhrases;
    QString rankSeparator = QStringLiteral("dash");
    QSet<QString> shortenedSizes;
    bool standardizeHaulingNames = false;

    bool operator==(const TagConfig &) const = default;

    QString toJson() const;
    static std::optional<TagConfig> fromJson(const QString &json);
    static TagConfig fromObject(const QJsonObject &object);
};

TagConfig defaultConfig(const QString &category);

void migrateMapping(const QString &category, TagConfig &config);
void backfillNewElements(const QString &category, TagConfig &config);

QString renderTag(const TagConfig &config, const QHash<QString, QString> &values);

QString joinTag(const QString &name, const QString &tag, const QString &placement);

bool routeEnabled(const TagConfig *config);
QString renderRoute(const QString &from, const QString &to, const QString &arrowKey, bool fromMany = false,
                    bool toMany = false);
QString applyMissionTitle(const QString &original, const QString &route, const TagConfig &config);

QString abbreviateTitle(const QString &title, const QSet<QString> &enabled = {},
                        const QString &rankSeparator = QStringLiteral("dash"),
                        bool standardizeHauling = false);

QString fingerprint(const QMap<QString, TagConfig> &configs, bool annotateMissionDescs);

}
