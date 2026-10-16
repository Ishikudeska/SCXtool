#pragma once

#include "core/text/IniFile.h"

#include <QString>

namespace core {

void stampJournalEntries(IniMap &merged, const IniMap &stock, const QString &appName, const QString &version);

void stampFrontendVersion(IniMap &merged, const QString &appName, const QString &version);

inline const QString kFrontendVersionKey = QStringLiteral("Frontend_PU_Version");

}
