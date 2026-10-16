#pragma once

#include "core/text/IniFile.h"

#include <QSet>
#include <QString>

namespace core {

QByteArray renderGameFile(const QString &baseIniText, const IniMap &merged);

QString writeGameFile(const QString &baseIniPath, const IniMap &merged, const QString &outputPath);

QString validateGameFile(const QString &writtenPath, const QSet<QString> &stockKeys);

}
