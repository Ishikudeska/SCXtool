#pragma once

#include <QString>
#include <QStringView>

namespace core {

namespace category {
inline const QString kShips = QStringLiteral("Ships");
inline const QString kShipItems = QStringLiteral("Ship Items");
inline const QString kGear = QStringLiteral("Gear");
inline const QString kMissions = QStringLiteral("Missions");
inline const QString kCommodities = QStringLiteral("Commodities");
inline const QString kJournal = QStringLiteral("Journal");
inline const QString kMedicalConsumables = QStringLiteral("Medical Consumables");
inline const QString kOther = QStringLiteral("Other");
}

enum class EntryStatus {
    Unmodified,
    Modified,
    Enhanced,
    New,
};

QString statusName(EntryStatus status);

QString extractCategory(const QString &key);

bool hasFpsWeaponWord(QStringView lowerKey);
bool hasArmorGearWord(QStringView lowerKey);
bool hasShipWeaponSize(const QString &key);

bool isShipNameKey(QStringView key);

struct StringEntry
{
    QString key;
    QString sourceFile;
    QString category;
    QString originalValue;
    QString customValue;
    EntryStatus status = EntryStatus::Unmodified;

    bool isModified() const { return !customValue.isEmpty() && customValue != originalValue; }

    bool isFavoritableShip() const { return category == category::kShips && isShipNameKey(key); }
};

}
