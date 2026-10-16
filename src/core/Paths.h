#pragma once

#include <QString>

namespace core {

class Settings;

struct PathRoots
{
    bool portable = false;
    QString portableRoot;
    QString documentsDir;
    QString localAppData;

    static PathRoots defaults();
};

class Paths
{
public:
    Paths(const Settings &settings, PathRoots roots = PathRoots::defaults());

    QString userDataRoot() const;
    QString channelDataDir() const;
    QString cacheDir() const;
    QString baseIni(const QString &language = {}) const;
    QString enhancementsDir(const QString &language = {}) const;
    QString userIni() const;
    QString backupsDir() const;
    QString logsDir() const;
    QString dataForgeCacheDir() const;

    QString channelInstallDir() const;
    QString p4kPath() const;
    QString gameGlobalIni(const QString &language = {}) const;

private:
    QString language(const QString &requested) const;

    const Settings &settings_;
    PathRoots roots_;
};

}
