#include "core/gamesettings/SettingsCatalog.h"

#include <QJsonArray>

namespace core::gamesettings {

namespace {

SettingKind kindFrom(const QString &type)
{
    if (type == u"toggle")
        return SettingKind::Toggle;
    if (type == u"slider")
        return SettingKind::Slider;
    if (type == u"choice")
        return SettingKind::Choice;
    if (type == u"text")
        return SettingKind::Text;
    return SettingKind::Number;
}

QString guessTab(const QString &name, const QJsonArray &rules)
{
    for (const QJsonValue &rule : rules) {
        const QJsonObject r = rule.toObject();
        const QString prefix = r.value(QLatin1StringView("prefix")).toString();
        if (name.startsWith(prefix, Qt::CaseInsensitive))
            return r.value(QLatin1StringView("tab")).toString();
    }
    return {};
}

QString description(const QJsonObject &s, const QString &labelRef, const TextLookup &text)
{
    const QString explicitRef = s.value(QLatin1StringView("description")).toString();
    if (!explicitRef.isEmpty())
        return resolveText(explicitRef, text);
    if (labelRef.startsWith(u'@') && text.game)
        return text.game(labelRef.mid(1) + QStringLiteral("Desc"));
    return {};
}

}

QString resolveText(const QString &ref, const TextLookup &text)
{
    if (ref.startsWith(u'@'))
        return text.game ? text.game(ref.mid(1)) : QString();
    if (ref.startsWith(u'='))
        return ref.mid(1);
    return text.app ? text.app(ref) : ref;
}

QString labelFromName(const QString &name)
{
    QString out;
    for (qsizetype i = 0; i < name.size(); ++i) {
        const QChar c = name[i];
        if (c == u'_') {
            out += u' ';
            continue;
        }
        if (i > 0) {
            const QChar prev = name[i - 1];
            const bool nextLower = i + 1 < name.size() && name[i + 1].isLower();
            if ((c.isUpper() && (prev.isLower() || (prev.isUpper() && nextLower))) ||
                (c.isDigit() && !prev.isDigit()))
                out += u' ';
        }
        out += c;
    }
    return out.simplified();
}

SettingsCatalog buildSettingsCatalog(const QJsonObject &definitions, const AttributeDefaults &defaults,
                                     const QStringList &extraNames, const TextLookup &text)
{
    SettingsCatalog catalog;
    catalog.version = defaults.version;

    QHash<QString, const AttributeDef *> known;
    for (const AttributeDef &def : defaults.attributes)
        known.insert(def.name, &def);

    for (const QJsonValue &t : definitions.value(QLatin1StringView("tabs")).toArray()) {
        const QJsonObject tab = t.toObject();
        catalog.tabs.push_back({tab.value(QLatin1StringView("id")).toString(),
                                resolveText(tab.value(QLatin1StringView("label")).toString(), text)});
    }

    QHash<QString, int> sectionIndex;
    auto addSection = [&](const QString &id, const QString &tab, const QString &label) {
        sectionIndex.insert(id, static_cast<int>(catalog.sections.size()));
        catalog.sections.push_back({id, tab, label, {}});
        return static_cast<int>(catalog.sections.size()) - 1;
    };
    auto addSetting = [&](SettingDef def, int section) {
        const int index = static_cast<int>(catalog.settings.size());
        catalog.byName.insert(def.name, index);
        catalog.sections[std::size_t(section)].settings.push_back(index);
        catalog.settings.push_back(std::move(def));
    };

    for (const QJsonValue &sv : definitions.value(QLatin1StringView("sections")).toArray()) {
        const QJsonObject s = sv.toObject();
        const QString id = s.value(QLatin1StringView("id")).toString();
        const QString tab = s.value(QLatin1StringView("tab")).toString();
        const int section =
            addSection(id, tab, resolveText(s.value(QLatin1StringView("label")).toString(), text));
        for (const QJsonValue &entry : s.value(QLatin1StringView("settings")).toArray()) {
            const QJsonObject e = entry.toObject();
            const QString name = e.value(QLatin1StringView("name")).toString();
            const auto found = known.constFind(name);
            if (found == known.cend() || catalog.byName.contains(name))
                continue;
            const AttributeDef &attr = **found;
            SettingDef def;
            def.name = name;
            def.cvar = attr.cvar;
            def.defaultValue = attr.defaultValue;
            def.tab = tab;
            def.section = id;
            def.curated = true;
            const QString labelRef = e.value(QLatin1StringView("label")).toString();
            def.label = labelRef.isEmpty() ? QString() : resolveText(labelRef, text);
            if (def.label.isEmpty())
                def.label = labelFromName(name);
            def.description = description(e, labelRef, text);
            def.kind = kindFrom(e.value(QLatin1StringView("type")).toString());
            def.min = e.value(QLatin1StringView("min")).toDouble(def.min);
            def.max = e.value(QLatin1StringView("max")).toDouble(def.max);
            def.step = e.value(QLatin1StringView("step")).toDouble(def.step);
            def.scale = e.value(QLatin1StringView("scale")).toDouble(def.scale);
            def.decimals = e.value(QLatin1StringView("decimals")).toInt(def.decimals);
            def.unit = resolveText(e.value(QLatin1StringView("unit")).toString(), text);
            def.invert = e.value(QLatin1StringView("invert")).toBool();
            def.restart = e.value(QLatin1StringView("restart")).toBool();
            const QJsonValue choiceRef = e.value(QLatin1StringView("choices"));
            const QJsonArray choices = choiceRef.isString()
                                           ? definitions.value(QLatin1StringView("choiceSets"))
                                                 .toObject()
                                                 .value(choiceRef.toString())
                                                 .toArray()
                                           : choiceRef.toArray();
            for (const QJsonValue &cv : choices) {
                const QJsonObject c = cv.toObject();
                QString label = resolveText(c.value(QLatin1StringView("label")).toString(), text);
                const QString value = c.value(QLatin1StringView("value")).toString();
                def.choices.append({value, label.isEmpty() ? value : label});
            }
            addSetting(std::move(def), section);
        }
    }

    const QJsonArray rules = definitions.value(QLatin1StringView("fallback")).toArray();
    const QString otherLabel =
        resolveText(definitions.value(QLatin1StringView("otherLabel")).toString(), text);
    auto placeOther = [&](SettingDef def) {
        QString tab = guessTab(def.name, rules);
        if (tab.isEmpty() && !catalog.tabs.empty())
            tab = catalog.tabs.front().id;
        const QString id = tab + QStringLiteral(".other");
        const int section =
            sectionIndex.contains(id) ? sectionIndex.value(id) : addSection(id, tab, otherLabel);
        def.tab = tab;
        def.section = id;
        addSetting(std::move(def), section);
    };

    for (const AttributeDef &attr : defaults.attributes) {
        if (catalog.byName.contains(attr.name))
            continue;
        SettingDef def;
        def.name = attr.name;
        def.cvar = attr.cvar;
        def.defaultValue = attr.defaultValue;
        def.label = labelFromName(attr.name);
        bool numeric = false;
        attr.defaultValue.toDouble(&numeric);
        def.kind = numeric ? SettingKind::Number : SettingKind::Text;
        placeOther(std::move(def));
    }
    for (const QString &name : extraNames) {
        if (catalog.byName.contains(name))
            continue;
        SettingDef def;
        def.name = name;
        def.label = labelFromName(name);
        def.kind = SettingKind::Text;
        def.inDefaults = false;
        placeOther(std::move(def));
    }
    return catalog;
}

bool toggleChecked(const SettingDef &def, const QString &value)
{
    bool ok = false;
    const double number = value.toDouble(&ok);
    const bool on = ok && number != 0.0;
    return def.invert ? !on : on;
}

QString toggleValue(const SettingDef &def, bool checked)
{
    return (checked != def.invert) ? QStringLiteral("1") : QStringLiteral("0");
}

}
