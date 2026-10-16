#pragma once

#include "core/model/StringEntry.h"
#include "core/text/IniFile.h"

#include <QDateTime>
#include <QFileInfoList>
#include <QList>
#include <QString>

#include <optional>

namespace core {

class UserIni
{
public:
    static constexpr int kKeepBackups = 5;

    explicit UserIni(QString path);

    const QString &path() const { return path_; }
    QString backupsDir() const;

    IniMap load() const;

    std::optional<int> save(const QList<StringEntry> &entries) const;
    std::optional<int> save(const IniMap &values) const;

    bool shouldAutosave(const QList<StringEntry> &entries) const;

    QString backup(const QDateTime &now = QDateTime::currentDateTime()) const;
    QFileInfoList backups() const;
    bool restore(const QString &backupFile) const;

    QString reset(bool keepBackup = true, const QDateTime &now = QDateTime::currentDateTime()) const;

    int generateFromDiff(const QString &referenceBaseIni, const QString &currentGameFile) const;

private:
    bool writeText(const QString &text) const;
    void backupIfChanging(const QString &newText) const;

    QString path_;
};

int migrateUserDataDir(const QString &oldRoot, const QString &newRoot, bool move = false);

}
