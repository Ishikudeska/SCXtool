#pragma once

#include <QByteArray>
#include <QJsonArray>
#include <QString>
#include <QUrl>

#include <array>
#include <atomic>
#include <functional>
#include <optional>

namespace core::net {

using Version = std::array<int, 3>;

std::optional<Version> parseVersion(const QString &text);

bool isNewer(const QString &latest, const QString &current);

struct InstallerAsset
{
    QUrl url;
    qint64 size = 0;
    QByteArray sha256;
};

std::optional<InstallerAsset> pickInstallerAsset(const QJsonArray &assets, const QString &exeName);

struct UpdateCheck
{
    enum class Status { UpToDate, Available, Failed, Disabled };
    Status status = Status::Failed;
    QString latest;
    QUrl releasePage;
    QString notes;
    std::optional<InstallerAsset> installer;
    QString error;
};

QUrl latestReleaseApi(const QString &repo);

UpdateCheck checkForUpdate(const QString &repo, const QString &currentVersion, int timeoutMs = 10'000);

using DownloadProgress = std::function<void(qint64 done, qint64 total)>;
QString downloadInstaller(const InstallerAsset &asset, const QString &destDir, QString *error,
                          const DownloadProgress &progress = {}, const std::atomic<bool> *cancel = nullptr);

}
