#pragma once

#include <QByteArray>
#include <QString>

#include <optional>

namespace core {

enum class IniEncoding {
    Utf8,
    Utf8Repaired,
    Windows1252,
};

struct IniText
{
    QString text;
    IniEncoding encoding = IniEncoding::Utf8;
    int replaced = 0;
};

IniText decodeIniText(const QByteArray &bytes);

std::optional<IniText> readIniText(const QString &path);

QString decodeWindows1252(QByteArrayView bytes);

}
