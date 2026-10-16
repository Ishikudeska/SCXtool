#pragma once

#include <QRegularExpression>
#include <QString>
#include <QStringView>

namespace core::py {

bool isSpace(QChar c);

QString strip(QStringView s);
QString lstrip(QStringView s);
QString rstrip(QStringView s);

QStringView rstrip(QStringView s, QChar c);

qsizetype len(QStringView s);

bool less(QStringView a, QStringView b);

QString capitalize(QStringView s);
QString title(QStringView s);

inline const QString kReSpace = QStringLiteral(R"([\s\x{1c}-\x{1f}\x{85}])");

QString rePattern(const QString &pattern);
QRegularExpression re(const QString &pattern, QRegularExpression::PatternOptions options = {});

}
