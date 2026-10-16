#include "core/blueprints/BlueprintMeta.h"

#include "core/tags/TagBuilder.h"
#include "core/text/PyText.h"

#include <QRegularExpression>

#include <array>

namespace core::blueprints {

namespace {

constexpr auto kUcp = QRegularExpression::UseUnicodePropertiesOption;
constexpr auto kCi = QRegularExpression::CaseInsensitiveOption;

constexpr QLatin1StringView kTypeFpsWeapon("FPS Weapon");
constexpr QLatin1StringView kTypeShipWeapon("Ship Weapon");
constexpr QLatin1StringView kTypeArmor("Armor");
constexpr QLatin1StringView kTypeAmmo("Ammo");
constexpr QLatin1StringView kTypeOther("Other");

constexpr std::array kArmorExtraWords = {u"backpack", u"undersuit", u"flightsuit", u"torso",    u"_legs",
                                         u"_arms",    u"_core",     u"gys_jacket", u"gys_pants"};

constexpr std::array kAmmoKeySuffixes = {u"_mag", u"_mag_empty", u"_mag_filled"};

constexpr std::array kExtraNameKeyPrefixes = {u"item_fuelnozzle_", u"nozzle_fuelgiver_",
                                              u"item_mining_mininglaser_"};

constexpr std::array kRawBlueprintFilenamePrefixes = {u"bp_craft_", u"bp_rewards_", u"bp_"};

const QHash<QString, QString> &typeLabels()
{
    static const QHash<QString, QString> labels = {
        {QStringLiteral("SHLD"), QStringLiteral("Shield")},
        {QStringLiteral("POWR"), QStringLiteral("Power Plant")},
        {QStringLiteral("COOL"), QStringLiteral("Cooler")},
        {QStringLiteral("QDRV"), QStringLiteral("Quantum Drive")},
        {QStringLiteral("QRDV"), QStringLiteral("Quantum Drive")},
        {QStringLiteral("JUMP"), QStringLiteral("Jump Drive")},
        {QStringLiteral("RADR"), QStringLiteral("Radar")},
        {QStringLiteral("MISL"), QStringLiteral("Missile")},
        {QStringLiteral("GMISL"), QStringLiteral("Guided Missile")},
        {QStringLiteral("BOMB"), QStringLiteral("Bomb")},
    };
    return labels;
}

constexpr std::pair<const char *, const char *> kManualItems[] = {
    {"Chiron Arms Purgatory Camo", "Armor"},
    {"Chiron Backpack Purgatory Camo", "Armor"},
    {"Chiron Core Purgatory Camo", "Armor"},
    {"Chiron Helmet Purgatory Camo", "Armor"},
    {"Chiron Legs Purgatory Camo", "Armor"},
    {"Testudo Arms Purgatory Camo", "Armor"},
    {"Testudo Backpack Purgatory Camo", "Armor"},
    {"Testudo Core Purgatory Camo", "Armor"},
    {"Testudo Helmet Purgatory Camo", "Armor"},
    {"Testudo Legs Purgatory Camo", "Armor"},
    {"Monde Arms Purgatory Camo", "Armor"},
    {"Monde Core Purgatory Camo", "Armor"},
    {"Monde Helmet Purgatory Camo", "Armor"},
    {"Monde Legs Purgatory Camo", "Armor"},
    {"Warden Backpack Purgatory Camo", "Armor"},
    {"Demeco \"Purgatory Camo\" LMG", "FPS Weapon"},
    {"S71 \"Purgatory Camo\" Rifle", "FPS Weapon"},
    {"BR-2 \"Purgatory Camo\" Shotgun", "FPS Weapon"},
    {"QuadraCell", "Power Plant"},
    {"QuadraCell MT", "Power Plant"},
    {"FR-66", "Shield"},
    {"FR-76", "Shield"},
    {"NDB-26 Repeater", "Ship Weapon"},
    {"NDB-28 Repeater", "Ship Weapon"},
    {"NDB-30 Repeater", "Ship Weapon"},
};

template <std::size_t N> bool startsWithAny(QStringView s, const std::array<const char16_t *, N> &prefixes)
{
    return std::any_of(prefixes.begin(), prefixes.end(),
                       [&](const char16_t *p) { return s.startsWith(QStringView(p)); });
}

template <std::size_t N> bool endsWithAny(QStringView s, const std::array<const char16_t *, N> &suffixes)
{
    return std::any_of(suffixes.begin(), suffixes.end(),
                       [&](const char16_t *p) { return s.endsWith(QStringView(p)); });
}

template <std::size_t N> bool containsAny(QStringView s, const std::array<const char16_t *, N> &words)
{
    return std::any_of(words.begin(), words.end(),
                       [&](const char16_t *w) { return s.contains(QStringView(w)); });
}

bool isNameKey(const QString &key)
{
    const QString kl = key.toLower();
    return kl.startsWith(u"item_name") || kl.startsWith(u"vehicle_name") ||
           startsWithAny(kl, kExtraNameKeyPrefixes);
}

QString sizeToken(QStringView digits)
{
    qsizetype i = 0;
    while (i + 1 < digits.size() && digits[i] == u'0')
        ++i;
    return QStringLiteral("S") + digits.sliced(i);
}

QString keySlug(const QString &key)
{
    QStringList segments = key.split(u'_');
    if (!segments.isEmpty() && segments.back().toLower() == u"name")
        segments.removeLast();
    for (QString &seg : segments)
        seg = py::capitalize(seg);
    return segments.join(u' ');
}

QString rawBlueprintFilenameSlug(const QString &rawName)
{
    const auto [stem, matched] = stripRawBlueprintFilenamePrefix(rawName);
    if (!matched)
        return QString();
    QString words = stem;
    words.replace(u'_', u' ').replace(u'-', u' ');
    return py::title(words);
}

struct ParsePatterns
{
    QRegularExpression leading;
    QRegularExpression trailing;
    bool valid = false;
};

const ParsePatterns &parsePatterns(const Enclosings &enclosings)
{
    thread_local QHash<QString, ParsePatterns> cache;
    QString key;
    for (const auto &[open, close] : enclosings)
        key += open + QChar(0x1F) + close + QChar(0x1E);
    if (const auto it = cache.constFind(key); it != cache.cend())
        return *it;

    QStringList alts;
    for (const auto &[open, close] : enclosings)
        if (!open.isEmpty() && !close.isEmpty())
            alts << QRegularExpression::escape(open) + QStringLiteral("([^") +
                        QRegularExpression::escape(close) + QStringLiteral("]+)") +
                        QRegularExpression::escape(close);
    ParsePatterns patterns;
    if (!alts.isEmpty()) {
        const QString alt = alts.join(u'|');
        const QString ws = py::kReSpace + u'*';
        patterns.leading =
            QRegularExpression(QStringLiteral("^") + ws + QStringLiteral("(?:") + alt + u')', kUcp);
        patterns.trailing = QRegularExpression(QStringLiteral("(?:") + alt + u')' + ws + u'$', kUcp);
        patterns.valid = true;
    }
    return *cache.insert(key, patterns);
}

QString firstCaptured(const QRegularExpressionMatch &m)
{
    for (int i = 1; i <= m.lastCapturedIndex(); ++i)
        if (m.hasCaptured(i))
            return m.captured(i);
    return QString();
}

QString pairKey(const QString &base, const QString &num)
{
    return base.toLower() + QChar(0x1F) + num;
}

struct Attrs
{
    QString cls, size, grade;
};

}

std::span<const std::pair<const char *, const char *>> manualBlueprintItems()
{
    return kManualItems;
}

QString expandClassFullWord(const QString &cls)
{
    static const QHash<QString, QString> full = [] {
        QHash<QString, QString> m;
        const tags::Mapping &classes = tags::defaultKindMappings().value(QStringLiteral("class"));
        for (const tags::Variants &v : classes)
            for (const QString &variant : v)
                m.insert(variant.toUpper(), v[2]);
        return m;
    }();
    if (cls.isEmpty())
        return cls;
    return full.value(cls.toUpper(), cls);
}

QString stripSizePrefix(const QString &size)
{
    if (!size.isEmpty() && size.front().toUpper() == u'S')
        return size.sliced(1);
    return size;
}

std::pair<QString, bool> stripRawBlueprintFilenamePrefix(const QString &stem)
{
    const QString lowered = stem.toLower();
    for (const char16_t *prefix : kRawBlueprintFilenamePrefixes) {
        const QStringView p(prefix);
        if (lowered.startsWith(p))
            return {stem.sliced(p.size()), true};
    }
    return {stem, false};
}

ComponentTag parseComponentTag(const QString &value, const Enclosings &enclosings, const QString &stock)
{
    std::optional<QString> inner;
    if (!stock.isEmpty())
        inner = stripViaStockDiff(value, stock);
    if (!inner) {
        if (const ParsePatterns &patterns = parsePatterns(enclosings); patterns.valid) {
            QRegularExpressionMatch m = patterns.leading.match(value);
            if (!m.hasMatch())
                m = patterns.trailing.match(value);
            if (m.hasMatch())
                inner = firstCaptured(m);
        }
    }
    if (!inner && enclosings.contains(kNoneStyleEnclosing)) {
        if (const auto found = findNoneStyleTagWord(value))
            inner = found->first;
    }
    ComponentTag tag;
    if (!inner)
        return tag;
    static const QRegularExpression nonAlnum(QStringLiteral("[^A-Za-z0-9]+"));
    static const QRegularExpression sizeToken_(QStringLiteral(R"(^S?(\d+)$)"), kUcp | kCi);
    for (const auto tokens = inner->split(nonAlnum, Qt::SkipEmptyParts); const QString &tok : tokens) {
        const QRegularExpressionMatch sm = sizeToken_.match(tok);
        if (sm.hasMatch() && tag.size.isEmpty())
            tag.size = sizeToken(sm.capturedView(1));
        else if (tok.size() == 1 && QStringLiteral("ABCDEF").contains(tok.toUpper()) && tag.grade.isEmpty())
            tag.grade = tok.toUpper();
        else if (tok.size() >= 2 && tag.cls.isEmpty() && !sm.hasMatch())
            tag.cls = tok.toUpper();
    }
    return tag;
}

QString sizeFromKey(const QString &key)
{
    static const QRegularExpression re(QStringLiteral(R"(_S0*(\d+)(?:_|$))"), kUcp | kCi);
    const QRegularExpressionMatch m = re.match(key);
    return m.hasMatch() ? sizeToken(m.capturedView(1)) : QString();
}

QString componentTypeFromKey(const QString &key)
{
    static const QRegularExpression re(QStringLiteral("^item_name_?([a-z]+)"), kCi);
    const QRegularExpressionMatch m = re.match(key);
    return m.hasMatch() ? typeLabels().value(m.captured(1).toUpper()) : QString();
}

QString blueprintTypeFromKey(const QString &key)
{
    if (key.isEmpty())
        return QString();
    if (const QString t = componentTypeFromKey(key); !t.isEmpty())
        return t;
    const QString kl = key.toLower();
    if (endsWithAny(kl, kAmmoKeySuffixes))
        return kTypeAmmo;
    if (hasFpsWeaponWord(kl))
        return kTypeFpsWeapon;
    if (hasArmorGearWord(kl) || containsAny(kl, kArmorExtraWords))
        return kTypeArmor;
    if (kl.contains(u"mining_head"))
        return QString();
    if (kl.startsWith(u"item_name")) {
        const QStringView after = QStringView(key).sliced(9);
        if (!after.isEmpty() && after.front().isUpper() && hasShipWeaponSize(key))
            return kTypeShipWeapon;
    }
    return QString();
}

QString cleanMissionTitle(const QString &value)
{
    if (value.isEmpty())
        return QString();
    static const QRegularExpression tags(
        QStringLiteral(R"((?:%1*<EM\d>\[[^\]]*\]</EM\d>)+%1*$)").arg(py::kReSpace), kUcp);
    const qsizetype nl = value.indexOf(QStringLiteral("\\n"));
    QString head = nl >= 0 ? value.first(nl) : value;
    head.replace(tags, QString());
    return py::strip(head);
}

QSet<QString> knownItemNames(const QList<StringEntry> &entries, const Enclosings &enclosings,
                             const QHash<QString, QString> &defaultValues)
{
    QSet<QString> names;
    for (const StringEntry &e : entries) {
        if (!isNameKey(e.key))
            continue;
        const QString name = normalizeItemName(e.originalValue, enclosings, defaultValues.value(e.key));
        if (!name.isEmpty())
            names.insert(name);
    }
    return names;
}

QMap<QString, BlueprintItem> buildBlueprintMetadata(const QList<StringEntry> &entries,
                                                    const Enclosings &enclosings,
                                                    const QHash<QString, QString> &defaultValues,
                                                    const QString &bpHeader)
{
    static const QRegularExpression titleKey(QStringLiteral(R"(^(.*)_Title(?:_(\d+))?$)"), kUcp | kCi);
    static const QRegularExpression descKey(QStringLiteral(R"(^(.*)_Desc(?:_(\d+))?$)"), kUcp | kCi);

    QHash<QString, QString> nameToKey;
    QHash<QString, QString> nameToValue;
    QHash<QString, Attrs> attrs;
    QHash<QString, QString> titles;
    QList<std::pair<QString, QString>> bpDescs;
    QHash<QString, QString> keySlugToName;

    for (const StringEntry &e : entries) {
        const QString &key = e.key;
        const QString &val = e.originalValue;

        if (isNameKey(key)) {
            const QString stock = defaultValues.value(key);
            const QString name = normalizeItemName(val, enclosings, stock);
            if (!name.isEmpty()) {
                keySlugToName.insert(keySlug(key), name);
                const QString trimmed = py::strip(val);
                const auto existing = nameToValue.constFind(name);
                if (existing == nameToValue.cend() || py::len(trimmed) > py::len(*existing)) {
                    nameToKey.insert(name, key);
                    nameToValue.insert(name, trimmed);
                    if (e.category == category::kShipItems && !componentTypeFromKey(key).isEmpty()) {
                        const ComponentTag tag = parseComponentTag(val, enclosings, stock);
                        const QString cls = expandClassFullWord(tag.cls);
                        const QString keySize = sizeFromKey(key);
                        const QString size = stripSizePrefix(keySize.isEmpty() ? tag.size : keySize);
                        if (!cls.isEmpty() || !size.isEmpty() || !tag.grade.isEmpty())
                            attrs.insert(name, {cls, size, tag.grade});
                        else
                            attrs.remove(name);
                    }
                }
            }
        }

        if (const QRegularExpressionMatch tm = titleKey.match(key); tm.hasMatch())
            titles.insert(pairKey(tm.captured(1), tm.captured(2)), cleanMissionTitle(val));

        if (e.category == category::kMissions && hasBpSection(val, bpHeader)) {
            const QRegularExpressionMatch dm = descKey.match(key);
            bpDescs.append({dm.hasMatch() ? pairKey(dm.captured(1), dm.captured(2)) : QString(), val});
        }
    }

    QHash<QString, QSet<QString>> missionsByName;
    const QHash<QString, QString> &aliases = bulletNameAliases();
    for (const auto &[pair, val] : std::as_const(bpDescs)) {
        const QString title = pair.isNull() ? QString() : titles.value(pair);
        for (const auto names = extractBpItemNames(val, enclosings, bpHeader); const QString &raw : names) {
            QString name;
            if (nameToValue.contains(raw)) {
                name = raw;
            } else if (aliases.contains(raw)) {
                name = aliases.value(raw);
            } else if (keySlugToName.contains(raw)) {
                name = keySlugToName.value(raw);
            } else {
                const QString slug = rawBlueprintFilenameSlug(raw);
                name = slug.isEmpty() ? raw : aliases.value(slug, raw);
            }
            QSet<QString> &bucket = missionsByName[name];
            if (!title.isEmpty())
                bucket.insert(title);
        }
    }

    const auto makeItem = [&](const QString &name, QSet<QString> missions, const QString &type) {
        const Attrs a = attrs.value(name);
        const QString tagged = nameToValue.value(name);
        return BlueprintItem{
            name, std::move(missions), type, a.cls, a.size, a.grade, tagged.isEmpty() ? name : tagged};
    };

    QMap<QString, BlueprintItem> result;
    for (auto it = missionsByName.cbegin(); it != missionsByName.cend(); ++it) {
        const QString type = blueprintTypeFromKey(nameToKey.value(it.key()));
        result.insert(it.key(), makeItem(it.key(), it.value(), type.isEmpty() ? kTypeOther : type));
    }

    for (const auto &[rawName, manualType] : kManualItems) {
        const QString name = normalizeItemName(QString::fromUtf8(rawName), enclosings);
        if (name.isEmpty() || result.contains(name))
            continue;
        const QString type = blueprintTypeFromKey(nameToKey.value(name));
        result.insert(name, makeItem(name, {kManualMissionLabel},
                                     type.isEmpty() ? QString::fromUtf8(manualType) : type));
    }
    return result;
}

}
