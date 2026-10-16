#pragma once

#include <QString>
#include <QStringView>

namespace core::py {

QString jsonString(QStringView s, bool ensureAscii);

inline QString jsonBool(bool b)
{
    return b ? QStringLiteral("true") : QStringLiteral("false");
}

}
