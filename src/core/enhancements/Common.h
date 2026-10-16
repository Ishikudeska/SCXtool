#pragma once

#include "core/enhancements/Xml.h"
#include "core/text/IniFile.h"

#include <QString>
#include <QStringView>

#include <optional>
#include <string_view>

namespace core::enh {

using Loc = IniMap;

inline const QString kNl = QStringLiteral("\\n");
inline const QString kEnhancementSeparator = QStringLiteral("\\n\\n--- STATS ---\\n");
inline const QString kEffectSeparator = QStringLiteral("\\n\\n--- EFFECT ---\\n");
inline const QString kStatsPrependSeparator = QStringLiteral("\\n\\n------\\n\\n");
inline constexpr double kOverheatPlaceholder = 450'000;
inline constexpr std::string_view kNullUuid = "00000000-0000-0000-0000-000000000000";

QString humanizeKey(QStringView key);

QString appendEnhancements(const QString &existing, const QString &block,
                           const QString &separator = kEnhancementSeparator, bool prepend = false);

QString fmt(const std::optional<std::string_view> &value, QStringView unit = {}, int decimals = 0);
QString fmt(std::optional<double> value, QStringView unit = {}, int decimals = 0);
inline QString fmt(std::string_view value, QStringView unit = {}, int decimals = 0)
{
    return fmt(std::optional(value), unit, decimals);
}

std::optional<double> toFloat(const std::optional<std::string_view> &value);
std::optional<double> toFloat(std::string_view value);

bool isSentinelLocRef(std::string_view ref);
bool isSentinelKey(QStringView key);
bool isPlaceholderText(QStringView s);

QString locKey(Node root);
QString locNameKey(Node root);
QString missionLocKey(Node root);

QString synthesizeDescription(Node root, const QString &xmlFile, const QString &key);

QString extractItemSize(QStringView cls);

std::string_view polyType(Node el);

QString recordClassName(Node root, const QString &fallback);
QString lastDotPart(std::string_view tag);

QString fileStem(const QString &path);

}
