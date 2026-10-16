#pragma once

#include "core/enhancements/Context.h"

#include <functional>

namespace core::enh {

using EnhancementFn = std::function<QString(Node root)>;

struct ScanOptions
{
    std::function<QString(Node)> locKeyFn;
    bool generateNameTags = false;
    NameTagger nameTagger;
    QString nameTagPlacement = QStringLiteral("prepend");
    QString separator = kEnhancementSeparator;
    bool captureAll = false;
    const Loc *tagLoc = nullptr;
    bool prepend = false;
};

Loc scanEntityDir(const RecordStore &store, const QString &relDir, const EnhancementFn &fn, const Loc &loc,
                  const ScanOptions &options = {});

std::pair<int, int> mirrorScitemSiblings(Loc &out, const Loc &loc);

Loc generateComponents(const Context &ctx);
Loc generateMissiles(const Context &ctx);
Loc generateShipWeapons(const Context &ctx);
Loc generateFpsWeapons(const Context &ctx);
Loc generateShips(const Context &ctx);
Loc generateMedicalConsumables(const Context &ctx);

}
