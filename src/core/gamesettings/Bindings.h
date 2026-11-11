#pragma once

#include "core/gamesettings/SettingsCatalog.h"

#include <QByteArray>
#include <QHash>
#include <QString>
#include <QStringList>

#include <optional>
#include <utility>
#include <vector>

namespace core::gamesettings {

enum class Device { Keyboard, Mouse, Gamepad, Joystick };

QString deviceId(Device device);
std::optional<Device> deviceFromId(const QString &id);
QString inputPrefix(Device device, int instance);
bool isMouseInput(const QString &input);
Device defaultDevice(const QString &attribute, const QString &input);

struct ParsedInput
{
    Device device = Device::Keyboard;
    int instance = 1;
    QString input;
};

std::optional<ParsedInput> parseInput(const QString &prefixed);
QString blankInput(const QString &input);

struct DefaultBind
{
    Device device = Device::Keyboard;
    QString input;
    QString activationMode;
    int multiTap = 0;
};

struct ActionDef
{
    QString map;
    QString name;
    QString label;
    QString description;
    QString activationMode;
    QString optionGroup;
    std::vector<DefaultBind> defaults;
};

struct ActionMapDef
{
    QString name;
    QString label;
    QString category;
    std::vector<ActionDef> actions;
};

struct OptionNode
{
    QString name;
    QString label;
    QString invertCvar;
    int depth = 0;
    int parent = -1;
    int showInvert = 0;
    int showCurve = 0;
    int showSensitivity = 0;
    std::optional<bool> invert;
    std::optional<double> exponent;
    std::vector<std::pair<double, double>> curve;
};

struct OptionTree
{
    QString type;
    int instances = 1;
    double sensitivityMin = 0.0;
    double sensitivityMax = 0.0;
    std::vector<OptionNode> nodes;

    int find(const QString &name) const;
    std::optional<bool> inheritedInvert(int node) const;
    std::optional<double> inheritedExponent(int node) const;
};

struct BindingsCatalog
{
    std::vector<ActionMapDef> maps;
    std::vector<OptionTree> optionTrees;
    QStringList activationModes;

    const OptionTree *tree(const QString &type) const;
};

BindingsCatalog parseDefaultProfile(const QByteArray &xml, const TextLookup &text, QString *error = nullptr);

class KeyLabels
{
public:
    static KeyLabels parse(const QByteArray &xml, const TextLookup &text);

    QString label(Device device, const QString &input) const;
    QString describe(Device device, const QString &input) const;
    QStringList inputs(Device device) const;

private:
    static QString table(Device device);
    QHash<QString, QString> labels_;
    QHash<QString, QStringList> inputs_;
};

QString keyNameFromQt(int qtKey, quint32 nativeScanCode, bool keypad);
QString mouseButtonName(int qtButton);
bool isModifierName(const QString &name);

}
