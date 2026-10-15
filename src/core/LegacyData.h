#pragma once

#include "core/Paths.h"

#include <QString>
#include <QStringList>

namespace core {

// Where the app's folders were under its former name and where they go now.
struct LegacyDataLocations
{
    QString settingsDirFrom; // folder that held settings.ini under the old name
    QString settingsDirTo;   // folder holding it now
    QString settingsFile;    // settings.ini in settingsDirTo
    PathRoots roots;
};

// The locations this install uses. Call after applyIdentity().
LegacyDataLocations legacyDataLocations();

// The app was called Smart Citizen++ (identity::kLegacyAppName) before it
// became SCXtool. Moves the old settings folder, Documents\<old> and
// %LOCALAPPDATA%\<old> to the current name, and repoints the user_data_dir
// and cache_dir settings when they name an "<old>" folder. A folder whose new
// location already exists is left where it is, and portable builds are
// skipped. Returns one line per folder moved or not movable, for the log.
QStringList migrateLegacyData(const LegacyDataLocations &where);

} // namespace core
