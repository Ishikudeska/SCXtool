#include "core/merge/Merger.h"

#include "core/text/PyText.h"

#include <QHash>

#include <array>

namespace core {

namespace {

constexpr std::array kComponentCodes = {u"shld", u"powr", u"cool",  u"qdrv",
                                        u"jump", u"misl", u"gmisl", u"bomb"};

bool isItemKey(const QString &key)
{
    return key.startsWith(u"item_name", Qt::CaseInsensitive) ||
           key.startsWith(u"item_desc", Qt::CaseInsensitive);
}

}

QString canonicalItemKey(const QString &key)
{
    QString k = key;
    if (k.endsWith(u"_scitem", Qt::CaseInsensitive))
        k.chop(7);
    k = k.toLower();
    k.remove(u'_');

    bool hasCode = false;
    for (const char16_t *code : kComponentCodes)
        hasCode = hasCode || k.contains(QStringView(code));
    if (!hasCode)
        return k;

    for (const char16_t *code : kComponentCodes)
        k.replace(QString::fromUtf16(code), QString(u'_') + QString::fromUtf16(code));
    const QStringList parts = k.split(u'_', Qt::SkipEmptyParts);
    return parts.join(u'_');
}

void syncKeyVariants(IniMap &merged, const QSet<QString> &userEditedKeys)
{
    QHash<QString, qsizetype> groupIndex;
    QList<QList<QString>> groups;
    for (const auto &[key, value] : merged) {
        if (!isItemKey(key))
            continue;
        const QString canonical = canonicalItemKey(key);
        qsizetype group = groups.size();
        if (const auto it = groupIndex.constFind(canonical); it != groupIndex.cend()) {
            group = *it;
        } else {
            groupIndex.insert(canonical, group);
            groups.append(QList<QString>());
        }
        groups[group].append(key);
    }

    for (const QList<QString> &variants : std::as_const(groups)) {
        if (variants.size() < 2)
            continue;
        QList<QString> candidates;
        for (const QString &v : variants)
            if (userEditedKeys.contains(v))
                candidates.append(v);
        if (candidates.isEmpty())
            candidates = variants;

        const QString *best = nullptr;
        qsizetype bestLength = -1;
        for (const QString &c : std::as_const(candidates)) {
            const QString *value = merged.find(c);
            const qsizetype length = py::len(*value);
            if (length > bestLength) {
                best = value;
                bestLength = length;
            }
        }
        const QString synced = *best;
        for (const QString &v : variants)
            merged.insert(v, synced);
    }
}

IniMap mergeSourcesByHierarchy(const SourceMap &sources, const QStringList &hierarchy,
                               const IniMap *userOverrides)
{
    IniMap result;
    for (const QString &name : hierarchy) {
        const auto it = sources.find(name);
        if (it == sources.end())
            continue;
        result.reserve(result.size() + it->second.size());
        for (const auto &[key, value] : it->second)
            result.insert(key, value);
    }

    QSet<QString> edited;
    if (userOverrides) {
        for (const auto &[key, value] : *userOverrides) {
            result.insert(key, value);
            edited.insert(key);
        }
    }
    syncKeyVariants(result, edited);
    return result;
}

}
