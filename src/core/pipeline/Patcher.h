#pragma once

#include "core/text/IniFile.h"

#include <QList>
#include <QString>
#include <QStringList>

namespace core {

struct PatchReport
{
    int patchesSeen = 0;
    int filesRewritten = 0;
    int editsApplied = 0;
    int editsAlreadyApplied = 0;
    int editsMismatched = 0;
    int editsUnmatched = 0;
    QStringList errors;

    QString summary() const;
};

PatchReport applyPatches(const QString &patchRoot, const QString &recordsRoot);

struct LocstringWorkaround
{
    QString target;
    QString appendFrom;
    QString separator;
    QString description;
    QString patchFile;
};

QList<LocstringWorkaround> loadLocstringWorkarounds(const QString &patchRoot);

int applyLocstringWorkarounds(IniMap &entries, const QList<LocstringWorkaround> &workarounds);

}
