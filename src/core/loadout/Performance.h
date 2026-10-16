#pragma once

#include "core/loadout/Loadout.h"

#include <QHash>
#include <QString>

#include <optional>
#include <vector>

namespace core::loadout {

struct Consumer
{
    QString key;
    QString label;
    QString category;
    int min = 0;
    int max = 0;
    int pips = 0;
};

struct PowerPlan
{
    QHash<QString, int> pips;
    bool nav = false;
};

std::vector<Consumer> consumers(const Loadout &loadout, const PowerPlan &plan);

double powerModifier(const Resources &resources, int pips);

struct WeaponFit
{
    QString path, name;
    QString group;
    int size = 0;
    Damage perShot;
    int pellets = 1;
    double alpha = 0, burst = 0, sustained = 0;
};

struct Totals
{
    std::vector<WeaponFit> weapons;
    struct Group
    {
        int guns = 0;
        double alpha = 0, burst = 0, sustained = 0;
    };
    Group pilot, manned, remote, pds;
    double alpha = 0, burstDps = 0, sustainedDps = 0;
    Damage burstByType;
    double weaponEfficiency = 1;
    int missiles = 0, bombs = 0;
    double missileDamage = 0, bombDamage = 0;

    double shieldHp = 0, shieldRegen = 0;
    int shieldGenerators = 0;
    Damage shieldResistance, shieldAbsorption;
    double armorHp = 0;
    Damage armorMultiplier{1, 1, 1, 1, 1, 1}, armorDeflection;
    double emMultiplier = 1, irMultiplier = 1, csMultiplier = 1;
    int decoys = 0, noise = 0;
    double hullHp = 0, vitalHp = 0;

    double powerOutput = 0;
    int powerUsed = 0;
    double coolantOutput = 0;
    double em = 0, ir = 0;

    std::optional<FlightStats> flight;
    double mainThrust = 0, retroThrust = 0, maneuverThrust = 0, vtolThrust = 0;
    double hullMass = 0, loadedMass = 0;
    double accelForward = 0;
    double hydrogenFuel = 0, quantumFuel = 0;

    std::optional<QuantumStats> quantum;
    double quantumRange = 0;
};

Totals computeTotals(const Loadout &loadout, const std::vector<Consumer> &consumers);

double quantumTravelTime(const QuantumStats &drive, double metres);

struct Engagement
{
    double shieldTime = 0;
    double killTime = 0;
    int deflected = 0;
};
Engagement engage(const Totals &attacker, const Totals &target);

}
