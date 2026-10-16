#pragma once

#include <QDateTime>
#include <QFileInfoList>
#include <QString>

#include <utility>

namespace core {

class GameFileBackups
{
public:
    static constexpr int kKeep = 5;

    explicit GameFileBackups(QString backupsDir) : dir_(std::move(backupsDir)) {}

    QString backup(const QString &gameFile, QString *error = nullptr,
                   const QDateTime &now = QDateTime::currentDateTime()) const;

    QFileInfoList list() const;

    static QString restore(const QString &backupFile, const QString &gameFile);

private:
    QString dir_;
};

}
