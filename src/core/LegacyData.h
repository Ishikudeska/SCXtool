#pragma once

#include "core/Paths.h"

#include <QString>
#include <QStringList>

namespace core {

struct LegacyDataLocations
{
    QString settingsDirFrom;
    QString settingsDirTo;
    QString settingsFile;
    PathRoots roots;
};

LegacyDataLocations legacyDataLocations();

QStringList migrateLegacyData(const LegacyDataLocations &where);

}
