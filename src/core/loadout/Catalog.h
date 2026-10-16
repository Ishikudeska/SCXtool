#pragma once

#include "core/text/IniFile.h"

#include <QHash>
#include <QString>
#include <QStringList>

#include <atomic>
#include <optional>
#include <vector>

namespace core::loadout {

struct Damage
{
    double physical = 0, energy = 0, distortion = 0, thermal = 0, biochemical = 0, stun = 0;

    double total() const { return physical + energy + distortion + thermal + biochemical + stun; }
    Damage operator*(double f) const
    {
        return {physical * f, energy * f, distortion * f, thermal * f, biochemical * f, stun * f};
    }
    Damage &operator+=(const Damage &o)
    {
        physical += o.physical;
        energy += o.energy;
        distortion += o.distortion;
        thermal += o.thermal;
        biochemical += o.biochemical;
        stun += o.stun;
        return *this;
    }
    bool operator==(const Damage &) const = default;
};

struct PortType
{
    QString type;
    QStringList subTypes;
};

struct Port
{
    QString name;
    QString label;
    int minSize = 0, maxSize = 0;
    std::vector<PortType> types;
    QStringList portTags;
    QStringList requiredTags;
    bool editable = true;
    bool hidden = false;
};

struct PowerRange
{
    double start = 0, modifier = 1;
};

struct Resources
{
    double powerDraw = 0;
    double powerMinFraction = 0;
    double powerOutput = 0;
    double coolantOutput = 0;
    double quantumFuel = 0;
    double em = 0, ir = 0;
    std::vector<PowerRange> ranges;
};

struct WeaponStats
{
    QString fireMode;
    double fireRate = 0;
    int pellets = 1;
    Damage damage;
    double speed = 0;
    double range = 0;
    double penetration = 0;
    int ammo = 0;
    bool regenerates = false;
    double regenPerSecond = 0;
    double regenCooldown = 0;
    double heatPerShot = 0;
    double overheatTemperature = 0;
    double overheatFixTime = 0;

    double alpha() const { return damage.total(); }
    double burstDps() const { return alpha() * fireRate / 60.0; }
    double timeToOverheat() const;
    double sustainedDps(double efficiency = 1) const;
};

struct MissileStats
{
    Damage damage;
    QString tracking;
    double lockTime = 0, lockAngle = 0, lockRangeMin = 0, lockRangeMax = 0;
    double speed = 0, armTime = 0, lifetime = 0;
};

struct ShieldStats
{
    double hp = 0, regen = 0, damagedDelay = 0, downedDelay = 0;
    Damage resistanceMin, resistanceMax;
    Damage absorptionMin, absorptionMax;
};

struct QuantumStats
{
    double speed = 0, splineSpeed = 0;
    double spoolTime = 0, cooldown = 0;
    double stageOneAccel = 0, stageTwoAccel = 0;
    double fuelPerGm = 0;
};

struct ThrusterStats
{
    QString kind;
    double thrust = 0;
};

struct ArmorStats
{
    Damage multiplier{1, 1, 1, 1, 1, 1};
    Damage deflection;
    double emMultiplier = 1, irMultiplier = 1, csMultiplier = 1;
};

struct FlightStats
{
    double scmSpeed = 0, boostForward = 0, boostBackward = 0, maxSpeed = 0;
    double pitch = 0, yaw = 0, roll = 0;
    double pitchBoost = 1, yawBoost = 1, rollBoost = 1;
    double boostCapacity = 0, boostRegen = 0, boostRegenDelay = 0;
};

struct CountermeasureStats
{
    QString kind;
    int ammo = 0;
};

struct LoadoutEntry
{
    QString port;
    QString item;
    QString refItem;
    std::vector<LoadoutEntry> children;
};

struct Item
{
    QString id;
    QString guid;
    QString name, shortName, description;
    QString manufacturer, manufacturerCode;
    QString type, subType;
    int size = 0, grade = 0;
    QString itemClass;
    QStringList tags, requiredTags;
    double mass = 0, health = 0;
    bool named = false;
    std::vector<Port> ports;
    std::vector<LoadoutEntry> loadout;
    Resources resources;
    std::optional<WeaponStats> weapon;
    std::optional<MissileStats> missile;
    std::optional<ShieldStats> shield;
    std::optional<QuantumStats> quantum;
    std::optional<ThrusterStats> thruster;
    std::optional<ArmorStats> armor;
    std::optional<FlightStats> flight;
    std::optional<CountermeasureStats> countermeasure;
    double fuelCapacity = 0;

    QString gradeLetter() const;
};

struct HullPart
{
    QString name;
    double hp = 0;
    bool vital = false;
};

struct PowerPool
{
    QString itemType;
    int size = 0;
    bool fixed = false;
};

struct Ship
{
    QString id;
    QString name, manufacturer, manufacturerCode, description, career, role;
    bool groundVehicle = false;
    int size = 0, crew = 0;
    double length = 0, width = 0, height = 0;
    double mass = 0;
    std::vector<HullPart> hull;
    std::vector<Port> ports;
    QStringList portTags;
    std::vector<LoadoutEntry> loadout;
    std::vector<PowerPool> powerPools;

    double hullHp() const;
    double vitalHp() const;
};

struct Catalog
{
    std::vector<Ship> ships;
    std::vector<Item> items;
    QHash<QString, int> byId;
    QHash<QString, int> byGuid;
    bool hasVehicles = false;

    const Item *item(const QString &id) const;
    const Ship *ship(const QString &id) const;
};

struct CatalogSources
{
    QString recordsDir;
    QString vehiclesDir;
    const IniMap *loc = nullptr;
};

Catalog buildCatalog(const CatalogSources &sources, const std::atomic<bool> *cancel = nullptr);

bool fits(const Port &port, const Item &item, const QStringList &shipTags);

bool isPlayerShipRecord(const QString &recordName);

}
