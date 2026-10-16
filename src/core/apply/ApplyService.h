#pragma once

#include "core/apply/UserCfg.h"
#include "core/merge/SourceLoader.h"
#include "core/model/StringEntry.h"
#include "core/text/IniFile.h"

#include <QList>
#include <QSet>
#include <QString>

#include <functional>

namespace core {

struct ApplyInputs
{
    LoadedSources sources;
    IniMap userOverrides;
    QString baseIniPath;
    QString gameFile;
    QString backupsDir;
    QString channelInstallDir;
    QString scLanguageId;
    QString appName;
    QString version;
    QSet<QString> excludeEnhancementKeys;
    QString languagesIniSource;
    QString languagesIniDest;
    std::function<void(IniMap &)> beforeStamps;
};

struct ApplyOutcome
{
    bool ok = false;
    QString error;
    QString validation;
    QString backupPath;
    bool restoredBackup = false;
    UserCfgResult userCfg = UserCfgResult::Failed;
};

IniMap userOverridesFrom(const QList<StringEntry> &entries);

ApplyOutcome applyToGame(const ApplyInputs &in);

}
