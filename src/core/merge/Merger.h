#pragma once

#include "core/text/IniFile.h"

#include <QSet>
#include <QString>
#include <QStringList>

#include <map>

namespace core {

using SourceMap = std::map<QString, IniMap>;

IniMap mergeSourcesByHierarchy(const SourceMap &sources, const QStringList &hierarchy,
                               const IniMap *userOverrides = nullptr);

void syncKeyVariants(IniMap &merged, const QSet<QString> &userEditedKeys = {});

QString canonicalItemKey(const QString &key);

}
