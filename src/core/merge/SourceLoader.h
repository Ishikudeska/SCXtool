#pragma once

#include "core/merge/Merger.h"
#include "core/model/StringEntry.h"

#include <QHash>
#include <QList>
#include <QString>
#include <QStringList>

namespace core {

inline const QString kSourceGlobal = QStringLiteral("global");
inline const QString kSourceEnhancements = QStringLiteral("enhancements");
inline const QString kSourceUser = QStringLiteral("user");

struct SourceFiles
{
    QString baseIni;
    QString enhancementsDir;
    QStringList enhancementFileIds;
    QString userIni;
};

struct LoadedSources
{
    SourceMap sources;
    QStringList hierarchy;
    QHash<QString, QString> enhancementCategories;
    QStringList problems;
};

LoadedSources loadSources(const SourceFiles &files);

QList<StringEntry> buildEntries(const LoadedSources &loaded, const IniMap *userOverrides = nullptr);

EntryStatus statusFromSource(const QString &source, const QString &baseSource, bool keyInBase,
                             bool inGlobalSource);

}
