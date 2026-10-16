#pragma once

#include <QString>
#include <QStringList>

namespace core {

bool isScInstallRoot(const QString &path);

bool endsInChannel(const QString &path);

QString normalizeInstallRoot(const QString &path);

QStringList installedChannels(const QString &root);

QStringList scanCommonInstallLocations();

QString pickLiveInstall(const QStringList &candidates);

QString locateScInstall();

}
