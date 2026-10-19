#pragma once

#include "core/pipeline/Patcher.h"
#include "engine/Error.h"
#include "engine/forge/DataForge.h"
#include "engine/p4k/Archive.h"

#include <QString>
#include <QStringList>

#include <atomic>
#include <functional>
#include <optional>

namespace core {

const QStringList &dataForgeKeepSubpaths();

struct P4kStamp
{
    qint64 size = -1;
    qint64 mtimeSecs = 0;

    static std::optional<P4kStamp> of(const QString &p4kPath);
    static std::optional<P4kStamp> read(const QString &stampFile);
    bool write(const QString &stampFile) const;
    QString key() const;
};

inline const QString kP4kStampName = QStringLiteral(".p4k_stamp");

using StepProgress = std::function<void(const QString &step, qint64 done, qint64 total)>;

engine::Result<void> extractBaseIni(const engine::p4k::Archive &archive, const QString &baseIniPath);

struct DataForgeExtraction
{
    std::size_t records = 0;
    PatchReport patches;
    std::size_t configFiles = 0;
    QString configError;
};

engine::Result<DataForgeExtraction> extractDataForge(const engine::p4k::Archive &archive,
                                                     const QString &cacheDir, const QString &patchRoot,
                                                     const StepProgress &progress = {},
                                                     const std::atomic<bool> *cancel = nullptr);

QString dataForgeRecordsDir(const QString &cacheDir);
QString dataForgeTagTablePath(const QString &cacheDir);
engine::Result<std::size_t> writeTagTable(const engine::forge::DataForge &forge, const QString &path);
QString dataForgeVehiclesDir(const QString &cacheDir);
engine::Result<std::size_t> writeVehicleDefinitions(const engine::p4k::Archive &archive, const QString &dir);

const QStringList &gameConfigEntries();
QString gameConfigDir(const QString &cacheDir);
engine::Result<std::size_t> extractGameConfig(const engine::p4k::Archive &archive, const QString &cacheDir);
bool gameConfigIsFresh(const QString &p4kPath, const QString &cacheDir);

bool baseIniIsFresh(const QString &p4kPath, const QString &baseIniPath);
bool dataForgeCacheIsFresh(const QString &p4kPath, const QString &cacheDir);

engine::Result<QStringList> exportGameData(const engine::p4k::Archive &archive, const QString &outputPath,
                                           const QString &baseIniPath, const QString &overlayPath = {},
                                           const QString &channel = {}, const StepProgress &progress = {});

}
