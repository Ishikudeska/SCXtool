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

    explicit GameFileBackups(QString backupsDir, QString prefix = QStringLiteral("global.ini.bak_"))
        : dir_(std::move(backupsDir)), prefix_(std::move(prefix))
    {}

    QString backup(const QString &gameFile, QString *error = nullptr,
                   const QDateTime &now = QDateTime::currentDateTime()) const;

    QFileInfoList list() const;

    static QString restore(const QString &backupFile, const QString &gameFile);

private:
    QString dir_;
    QString prefix_;
};

}
