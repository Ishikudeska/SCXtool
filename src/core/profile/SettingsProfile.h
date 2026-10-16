#pragma once

#include "core/ScInstall.h"
#include "core/Settings.h"

#include <QDate>
#include <QDateTime>
#include <QMap>
#include <QString>
#include <QVariantMap>

#include <expected>
#include <functional>

namespace core::profile {

inline constexpr int kSchemaVersion = 1;

inline const QString kSourcePortable = QStringLiteral("portable");
inline const QString kSourceInstalled = QStringLiteral("registry");

struct ProfileContents
{
    QVariantMap settings;
    QMap<QString, QString> overrides;
    int schemaVersion = kSchemaVersion;
    QString appVersion;
    QString exportedAt;
    QString sourceMode;
};

QString defaultBackupFilename(QDate today = QDate::currentDate());

std::expected<int, QString> writeProfileZip(const QString &zipPath, const QVariantMap &settings,
                                            const QMap<QString, QString> &overrides,
                                            const QString &appVersion, const QString &sourceMode,
                                            const QDateTime &now = QDateTime::currentDateTime());

std::expected<ProfileContents, QString> readProfileZip(const QString &zipPath);

bool isProfileExcludedKey(const QString &key, const QVariant &value = {});

QVariantMap exportSettingsValues(const Settings &settings);
int importSettingsValues(Settings &settings, const QVariantMap &values);

enum class InstallPathOutcome {
    Restored,
    Redetected,
    None,
};
InstallPathOutcome reconcileImportedInstallPath(Settings &settings,
                                                const std::function<QString()> &detect = locateScInstall);

}
