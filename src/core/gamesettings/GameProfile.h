#pragma once

#include <QString>
#include <QStringList>

namespace core::gamesettings {

struct ProfilePaths
{
    QString dir;
    QString attributes;
    QString actionMaps;
    QString mappingsDir;
};

QString resolveCaseInsensitive(const QString &base, const QStringList &parts);

ProfilePaths profilePaths(const QString &channelInstallDir);

bool isGameRunning();

}
