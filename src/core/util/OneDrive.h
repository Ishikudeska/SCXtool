#pragma once

#include <QProcessEnvironment>
#include <QString>
#include <QStringList>

namespace core::onedrive {

QStringList roots(const QProcessEnvironment &env = QProcessEnvironment::systemEnvironment());

bool isOneDrivePath(const QString &path,
                    const QProcessEnvironment &env = QProcessEnvironment::systemEnvironment());

QString suggestLocalDataDir(const QString &appName,
                            const QProcessEnvironment &env = QProcessEnvironment::systemEnvironment());

}
