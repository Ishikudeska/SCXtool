#pragma once

#include "core/gamesettings/Attributes.h"

#include <QHash>
#include <QJsonObject>
#include <QList>
#include <QString>
#include <QStringList>

#include <functional>
#include <vector>

namespace core::gamesettings {

enum class SettingKind { Toggle, Slider, Choice, Number, Text };

struct SettingChoice
{
    QString value;
    QString label;
};

struct SettingDef
{
    QString name;
    QString cvar;
    QString defaultValue;
    QString tab;
    QString section;
    QString label;
    QString description;
    QString unit;
    SettingKind kind = SettingKind::Number;
    double min = 0.0;
    double max = 1.0;
    double step = 0.0;
    double scale = 1.0;
    int decimals = 2;
    bool invert = false;
    bool restart = false;
    bool curated = false;
    bool inDefaults = true;
    QList<SettingChoice> choices;
};

struct SettingsSection
{
    QString id;
    QString tab;
    QString label;
    std::vector<int> settings;
};

struct SettingsTab
{
    QString id;
    QString label;
};

struct SettingsCatalog
{
    QString version;
    std::vector<SettingsTab> tabs;
    std::vector<SettingsSection> sections;
    std::vector<SettingDef> settings;
    QHash<QString, int> byName;

    int find(const QString &name) const { return byName.value(name, -1); }
};

struct TextLookup
{
    std::function<QString(const QString &key)> game;
    std::function<QString(const QString &key)> app;
};

QString resolveText(const QString &ref, const TextLookup &text);

QString labelFromName(const QString &name);

SettingsCatalog buildSettingsCatalog(const QJsonObject &definitions, const AttributeDefaults &defaults,
                                     const QStringList &extraNames, const TextLookup &text);

bool toggleChecked(const SettingDef &def, const QString &value);

QString toggleValue(const SettingDef &def, bool checked);

}
