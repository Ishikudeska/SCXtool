#pragma once

#include <QString>

namespace core::crash {

void install(const QString &logsDir);
bool isInstalled();

QString writeCrashReport(const QString &logsDir, const QString &reason, const QString &thread = {});

void reportFatal(const QString &reason);

}
