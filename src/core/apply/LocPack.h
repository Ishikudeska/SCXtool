#pragma once

#include <QDate>
#include <QString>

#include <expected>

namespace core {

QString defaultLocPackFilename(const QString &channel, QDate today = QDate::currentDate());

std::expected<qint64, QString> writeLocPackZip(const QString &sourceGlobalIni, const QString &outputZip);

}
