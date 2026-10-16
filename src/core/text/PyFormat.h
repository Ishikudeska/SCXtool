#pragma once

#include <QString>
#include <QStringView>

#include <optional>

namespace core::py {

std::optional<double> toFloat(QStringView s);
std::optional<qint64> toInt(QStringView s);

QString repr(double v);

double round(double v);

QString fixed(double v, int decimals, bool thousands = false, bool plus = false);
QString integer(qint64 n, bool thousands = false);
QString groupedRepr(double v);

QString groupThousands(const QString &number);

}
