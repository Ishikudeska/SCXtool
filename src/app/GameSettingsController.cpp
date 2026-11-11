#include "GameSettingsController.h"

#include "AppController.h"
#include "core/EnginePaths.h"
#include "core/apply/Backups.h"
#include "core/i18n/Translator.h"
#include "core/pipeline/Extraction.h"
#include "core/text/IniFile.h"
#include "engine/p4k/Archive.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QLocale>
#include <QLoggingCategory>
#include <QSet>

#include <algorithm>
#include <cmath>

Q_DECLARE_LOGGING_CATEGORY(lcApp)

using namespace core::gamesettings;

namespace {

QString text(const char *key, const QVariantHash &args = {})
{
    return core::i18n::tr(key, args);
}

constexpr QLatin1StringView kAttributesBackup("attributes.xml.bak_");
constexpr QLatin1StringView kActionMapsBackup("actionmaps.xml.bak_");
const QStringList kAxes = {QStringLiteral("x"),       QStringLiteral("y"),      QStringLiteral("z"),
                           QStringLiteral("rotx"),    QStringLiteral("roty"),   QStringLiteral("rotz"),
                           QStringLiteral("slider1"), QStringLiteral("slider2")};

QString kindName(SettingKind kind)
{
    switch (kind) {
    case SettingKind::Toggle:
        return QStringLiteral("toggle");
    case SettingKind::Slider:
        return QStringLiteral("slider");
    case SettingKind::Choice:
        return QStringLiteral("choice");
    case SettingKind::Text:
        return QStringLiteral("text");
    case SettingKind::Number:
        break;
    }
    return QStringLiteral("number");
}

bool matches(const QString &needle, std::initializer_list<QString> haystack)
{
    if (needle.isEmpty())
        return true;
    for (const QString &h : haystack)
        if (h.contains(needle, Qt::CaseInsensitive))
            return true;
    return false;
}

QByteArray readFile(const QString &path)
{
    QFile file(path);
    return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
}

QString configFile(const QString &cache, const char *name)
{
    return QDir(core::gameConfigDir(cache)).filePath(QString::fromLatin1(name));
}

bool configPresent(const QString &cache)
{
    for (const QString &entry : core::gameConfigEntries())
        if (!QFileInfo::exists(QDir(core::gameConfigDir(cache)).filePath(QFileInfo(entry).fileName())))
            return false;
    return true;
}

}

struct GameSettingsController::Loaded
{
    SettingsCatalog settings;
    BindingsCatalog bindings;
    KeyLabels keys;
    AttributesFile attributes;
    ActionMapsFile actionMaps;
    QString error;
};

GameSettingsController::GameSettingsController(QObject *parent) : QObject(parent)
{
    connect(&app(), &AppController::pathsChanged, this, &GameSettingsController::sourcesChanged);
    connect(&app(), &AppController::languageChanged, this, &GameSettingsController::sourcesChanged);
    connect(&app(), &AppController::channelChanged, this, &GameSettingsController::sourcesChanged);
    connect(&app(), &AppController::installChanged, this, &GameSettingsController::sourcesChanged);
    runningTimer_.setInterval(5000);
    connect(&runningTimer_, &QTimer::timeout, this, &GameSettingsController::refreshGameRunning);
}

AppController &GameSettingsController::app() const
{
    return *AppController::instance();
}

void GameSettingsController::setActive(bool active)
{
    if (active_ == active)
        return;
    active_ = active;
    emit activeChanged();
    if (active_) {
        refreshGameRunning();
        runningTimer_.start();
    } else {
        runningTimer_.stop();
    }
    sourcesChanged();
}

QString GameSettingsController::sourceKey() const
{
    return QStringList{app().paths().channelInstallDir(), app().dataForgeDir(), app().baseIniPath(),
                       app().language()}
        .join(u'|');
}

void GameSettingsController::sourcesChanged()
{
    if (!active_ || building_ || sourceKey() == builtKey_)
        return;
    if (pendingCount_ > 0)
        app().prompts()->info(text("game_settings.discarded_title"), text("game_settings.discarded_body"));
    extractTried_ = false;
    build();
}

void GameSettingsController::reload()
{
    if (building_)
        return;
    builtKey_.clear();
    build();
}

void GameSettingsController::setStatus(const QString &status, const QString &error)
{
    if (status_ == status && error_ == error)
        return;
    status_ = status;
    error_ = error;
    emit statusChanged();
}

void GameSettingsController::extractGameData()
{
    const QString p4k = app().p4kPath();
    const QString cache = app().dataForgeDir();
    if (p4k.isEmpty() || !QFileInfo::exists(p4k)) {
        setStatus(QStringLiteral("nodata"));
        return;
    }
    extractTried_ = true;
    building_ = true;
    setStatus(QStringLiteral("loading"));
    app().tasks()->run<QString>(
        text("game_settings.extracting"), false,
        [p4k, cache](TaskRunner::Job &job) -> QString {
            job.report(text("game_settings.extracting"));
            auto archive = engine::p4k::Archive::open(core::fsPath(p4k));
            if (!archive)
                return QString::fromStdString(archive.error().message);
            auto result = core::extractGameConfig(**archive, cache);
            if (!result)
                return QString::fromStdString(result.error().message);
            return {};
        },
        [this](const QString &error) {
            building_ = false;
            if (!error.isEmpty()) {
                setStatus(QStringLiteral("error"), error);
                return;
            }
            builtKey_.clear();
            build();
        });
}

void GameSettingsController::build()
{
    builtKey_ = sourceKey();
    const QString install = app().paths().channelInstallDir();
    if (install.isEmpty() || !QFileInfo(install).isDir()) {
        loaded_.reset();
        profile_ = {};
        setStatus(QStringLiteral("noinstall"));
        return;
    }
    profile_ = profilePaths(install);
    const QString cache = app().dataForgeDir();
    const QString p4k = app().p4kPath();
    const bool present = configPresent(cache);
    const bool stale = QFileInfo::exists(p4k) && !core::gameConfigIsFresh(p4k, cache);
    if ((!present || stale) && QFileInfo::exists(p4k) && !extractTried_) {
        extractGameData();
        return;
    }
    if (!present) {
        loaded_.reset();
        setStatus(QStringLiteral("nodata"));
        return;
    }

    building_ = true;
    setStatus(QStringLiteral("loading"));
    const QString baseIni = QFileInfo::exists(app().baseIniPath())
                                ? app().baseIniPath()
                                : app().paths().baseIni(core::kDefaultLanguage);
    const core::gamesettings::ProfilePaths profile = profile_;
    app().tasks()->run<std::shared_ptr<const Loaded>>(
        text("game_settings.loading"), false,
        [cache, baseIni, profile](TaskRunner::Job &job) -> std::shared_ptr<const Loaded> {
            job.report(text("game_settings.loading"));
            auto out = std::make_shared<Loaded>();
            QHash<QString, QString> loc;
            for (const auto &[key, value] : core::loadIni(baseIni)) {
                QString k = key.toLower();
                if (k.endsWith(QLatin1StringView(",p")))
                    k.chop(2);
                loc.insert(k, value);
            }
            TextLookup lookup;
            lookup.game = [&loc](const QString &key) { return loc.value(key.toLower()); };
            lookup.app = [](const QString &key) { return core::i18n::tr(key.toUtf8().constData()); };

            QString error;
            const AttributeDefaults defaults =
                parseAttributeDefaults(readFile(configFile(cache, "attributes.xml")), &error);
            if (!error.isEmpty()) {
                out->error = error;
                return out;
            }
            if (QFileInfo::exists(profile.attributes)) {
                auto file = AttributesFile::load(profile.attributes, &error);
                if (!file) {
                    out->error = QDir::toNativeSeparators(profile.attributes) + QStringLiteral(": ") + error;
                    return out;
                }
                out->attributes = std::move(*file);
            } else {
                out->attributes.setVersion(defaults.version);
            }
            if (QFileInfo::exists(profile.actionMaps)) {
                auto file = ActionMapsFile::load(profile.actionMaps, &error);
                if (!file) {
                    out->error = QDir::toNativeSeparators(profile.actionMaps) + QStringLiteral(": ") + error;
                    return out;
                }
                out->actionMaps = std::move(*file);
            } else {
                out->actionMaps = ActionMapsFile::empty();
            }

            QStringList extra;
            QSet<QString> known;
            for (const AttributeDef &def : defaults.attributes)
                known.insert(def.name);
            for (auto it = out->attributes.values().cbegin(); it != out->attributes.values().cend(); ++it)
                if (!known.contains(it.key()))
                    extra << it.key();

            const QJsonObject definitions =
                QJsonDocument::fromJson(readFile(QStringLiteral(":/gamesettings/settings.json"))).object();
            out->settings = buildSettingsCatalog(definitions, defaults, extra, lookup);
            out->bindings =
                parseDefaultProfile(readFile(configFile(cache, "defaultProfile.xml")), lookup, &error);
            if (!error.isEmpty())
                out->error = error;
            out->keys = KeyLabels::parse(readFile(configFile(cache, "keybinding_localization.xml")), lookup);
            return out;
        },
        [this](std::shared_ptr<const Loaded> loaded) {
            building_ = false;
            if (!loaded || !loaded->error.isEmpty()) {
                setStatus(QStringLiteral("error"), loaded ? loaded->error : QString());
                return;
            }
            loaded_ = std::move(loaded);
            attributes_ = loaded_->attributes;
            savedAttributes_ = loaded_->attributes;
            actionMaps_ = loaded_->actionMaps;
            savedActionMaps_ = loaded_->actionMaps;
            qCInfo(lcApp) << "game settings:" << loaded_->settings.settings.size() << "settings,"
                          << loaded_->bindings.maps.size() << "action maps";
            optionDevice_ = 0;
            emit catalogChanged();
            emit filtersChanged();
            edited();
            refreshRows();
            setStatus(QStringLiteral("ready"));
            sourcesChanged();
        });
}

QVariantList GameSettingsController::tabs() const
{
    QVariantList out;
    if (!loaded_)
        return out;
    for (const SettingsTab &t : loaded_->settings.tabs)
        out << QVariantMap{{QStringLiteral("id"), t.id}, {QStringLiteral("title"), t.label}};
    return out;
}

void GameSettingsController::setTab(const QString &tab)
{
    if (tab_ == tab)
        return;
    tab_ = tab;
    emit tabChanged();
    refreshRows();
}

void GameSettingsController::setSearch(const QString &search)
{
    if (search_ == search)
        return;
    search_ = search;
    emit filtersChanged();
    refreshRows();
}

void GameSettingsController::setModifiedOnly(bool on)
{
    if (modifiedOnly_ == on)
        return;
    modifiedOnly_ = on;
    emit filtersChanged();
    refreshRows();
}

void GameSettingsController::edited()
{
    int count = 0;
    if (attributes_ && savedAttributes_) {
        QSet<QString> names;
        for (auto it = attributes_->values().cbegin(); it != attributes_->values().cend(); ++it)
            names.insert(it.key());
        for (auto it = savedAttributes_->values().cbegin(); it != savedAttributes_->values().cend(); ++it)
            names.insert(it.key());
        for (const QString &name : std::as_const(names))
            if (attributes_->value(name) != savedAttributes_->value(name))
                ++count;
    }
    if (actionMaps_ && savedActionMaps_ && actionMaps_->serialize() != savedActionMaps_->serialize()) {
        auto keys = [](const ActionMapsFile &file) {
            QSet<QString> out;
            for (const BoundAction &b : file.rebinds())
                out.insert(QStringList{b.map, b.action, deviceId(b.device), QString::number(b.instance),
                                       b.rebind.input, b.rebind.activationMode,
                                       QString::number(b.rebind.multiTap)}
                               .join(u'|'));
            return out;
        };
        const QSet<QString> now = keys(*actionMaps_), before = keys(*savedActionMaps_);
        const int bindChanges = static_cast<int>((now - before).size() + (before - now).size());
        count += std::max(1, bindChanges);
    }
    if (pendingCount_ != count) {
        pendingCount_ = count;
        emit pendingChanged();
    }
    refreshBindUsage();
    ++revision_;
    emit revisionChanged();
    if (modifiedOnly_)
        refreshRows();
}

void GameSettingsController::refreshRows()
{
    refreshBindUsage();
    refreshSettingRows();
    refreshBindRows();
    refreshOptionRows();
    emit rowsChanged();
}

QString GameSettingsController::effectiveValue(const SettingDef &def) const
{
    if (attributes_)
        if (const auto v = attributes_->value(def.name))
            return *v;
    return def.defaultValue;
}

QVariantMap GameSettingsController::settingRow(const SettingDef &def) const
{
    const QString value = effectiveValue(def);
    const std::optional<QString> saved = savedAttributes_ ? savedAttributes_->value(def.name) : std::nullopt;
    const std::optional<QString> current = attributes_ ? attributes_->value(def.name) : std::nullopt;
    const QLocale locale;
    auto numberText = [&def](const QString &v) {
        bool ok = false;
        const double d = v.toDouble(&ok);
        if (!ok)
            return v;
        const QString shown = def.scale != 1.0 ? formatNumber(std::round(d * def.scale * 1000) / 1000) : v;
        return def.unit.isEmpty() ? shown : shown + u' ' + def.unit;
    };

    QVariantList choices;
    int choiceIndex = -1;
    QString defaultText = def.defaultValue;
    for (const SettingChoice &c : def.choices) {
        if (sameValue(c.value, value))
            choiceIndex = static_cast<int>(choices.size());
        if (sameValue(c.value, def.defaultValue))
            defaultText = c.label;
        choices << QVariantMap{{QStringLiteral("value"), c.value}, {QStringLiteral("label"), c.label}};
    }
    if (def.kind == SettingKind::Choice && choiceIndex < 0) {
        choiceIndex = static_cast<int>(choices.size());
        choices << QVariantMap{{QStringLiteral("value"), value},
                               {QStringLiteral("label"),
                                text("game_settings.unknown_value", {{QStringLiteral("value"), value}})}};
    }
    if (def.kind == SettingKind::Toggle)
        defaultText =
            toggleChecked(def, def.defaultValue) ? text("game_settings.on") : text("game_settings.off");
    else if (def.kind != SettingKind::Choice)
        defaultText = def.inDefaults ? numberText(def.defaultValue) : QString();

    bool ok = false;
    const double number = value.toDouble(&ok);
    return {
        {QStringLiteral("name"), def.name},
        {QStringLiteral("label"), def.label},
        {QStringLiteral("description"), def.description},
        {QStringLiteral("cvar"), def.cvar},
        {QStringLiteral("kind"), kindName(def.kind)},
        {QStringLiteral("value"), value},
        {QStringLiteral("checked"), toggleChecked(def, value)},
        {QStringLiteral("number"), ok ? number * def.scale : 0.0},
        {QStringLiteral("min"), def.min * def.scale},
        {QStringLiteral("max"), def.max * def.scale},
        {QStringLiteral("step"), def.step * def.scale},
        {QStringLiteral("scale"), def.scale},
        {QStringLiteral("decimals"), def.decimals},
        {QStringLiteral("unit"), def.unit},
        {QStringLiteral("choices"), choices},
        {QStringLiteral("choiceIndex"), choiceIndex},
        {QStringLiteral("defaultText"), defaultText},
        {QStringLiteral("modified"),
         def.inDefaults ? !sameValue(value, def.defaultValue) : current.has_value()},
        {QStringLiteral("pending"), current != saved},
        {QStringLiteral("restart"), def.restart},
        {QStringLiteral("curated"), def.curated},
    };
}

void GameSettingsController::refreshSettingRows()
{
    sections_.clear();
    if (!loaded_ || tab_ == u"keybinds")
        return;
    const SettingsCatalog &catalog = loaded_->settings;
    for (const SettingsSection &section : catalog.sections) {
        if (section.tab != tab_)
            continue;
        QVariantList rows;
        for (const int index : section.settings) {
            const SettingDef &def = catalog.settings[std::size_t(index)];
            if (!matches(search_, {def.label, def.name, def.cvar, def.description}))
                continue;
            QVariantMap row = settingRow(def);
            if (modifiedOnly_ && !row.value(QStringLiteral("modified")).toBool() &&
                !row.value(QStringLiteral("pending")).toBool())
                continue;
            rows << row;
        }
        if (!rows.isEmpty())
            sections_ << QVariantMap{{QStringLiteral("id"), section.id},
                                     {QStringLiteral("title"), section.label},
                                     {QStringLiteral("rows"), rows}};
    }
}

void GameSettingsController::setValue(const QString &name, const QString &value)
{
    if (!loaded_ || !attributes_ || !savedAttributes_)
        return;
    const int index = loaded_->settings.find(name);
    if (index < 0)
        return;
    const SettingDef &def = loaded_->settings.settings[std::size_t(index)];
    QString stored = value;
    if (def.kind == SettingKind::Slider || def.kind == SettingKind::Number) {
        bool ok = false;
        const double d = value.toDouble(&ok);
        if (!ok)
            return;
        stored = formatNumber(def.kind == SettingKind::Slider && def.scale != 0.0 ? d / def.scale : d);
    }
    const std::optional<QString> saved = savedAttributes_->value(name);
    if (saved && sameValue(*saved, stored))
        attributes_->set(name, *saved);
    else if (!saved && def.inDefaults && sameValue(stored, def.defaultValue))
        attributes_->remove(name);
    else
        attributes_->set(name, stored);
    edited();
}

void GameSettingsController::setToggle(const QString &name, bool checked)
{
    if (!loaded_)
        return;
    const int index = loaded_->settings.find(name);
    if (index >= 0)
        setValue(name, toggleValue(loaded_->settings.settings[std::size_t(index)], checked));
}

void GameSettingsController::resetValue(const QString &name)
{
    if (!attributes_)
        return;
    attributes_->remove(name);
    edited();
}

void GameSettingsController::resetTab()
{
    if (!loaded_ || !attributes_)
        return;
    if (tab_ == u"keybinds") {
        resetDeviceBindings();
        return;
    }
    for (const SettingDef &def : loaded_->settings.settings)
        if (def.tab == tab_ && def.inDefaults)
            attributes_->remove(def.name);
    if (tab_ == u"controls" && actionMaps_) {
        for (const OptionTarget &target : optionTargets()) {
            if (const OptionTree *tree = loaded_->bindings.tree(target.type))
                for (const OptionNode &node : tree->nodes) {
                    actionMaps_->removeOption(target.type, target.instance, node.name,
                                              QStringLiteral("invert"));
                    actionMaps_->removeOption(target.type, target.instance, node.name,
                                              QStringLiteral("exponent"));
                    actionMaps_->removeCurve(target.type, target.instance, node.name);
                }
            if (!target.product.isEmpty())
                for (const QString &axis : kAxes) {
                    actionMaps_->removeDeviceOption(target.product, axis, QStringLiteral("deadzone"));
                    actionMaps_->removeDeviceOption(target.product, axis, QStringLiteral("saturation"));
                }
        }
    }
    edited();
}

void GameSettingsController::discard()
{
    if (savedAttributes_)
        attributes_ = savedAttributes_;
    if (savedActionMaps_)
        actionMaps_ = savedActionMaps_;
    edited();
}

void GameSettingsController::refreshGameRunning()
{
    const bool running = isGameRunning();
    if (running != gameRunning_) {
        gameRunning_ = running;
        emit gameRunningChanged();
    }
}

bool GameSettingsController::writeFiles(QString *error)
{
    const core::GameFileBackups attributeBackups(app().paths().backupsDir(), kAttributesBackup);
    const core::GameFileBackups actionBackups(app().paths().backupsDir(), kActionMapsBackup);
    auto write = [&](const QString &path, const QByteArray &bytes, const core::GameFileBackups &backups,
                     auto &&saveFn) {
        if (QFileInfo::exists(path) && backups.backup(path, error).isEmpty())
            return false;
        if (!saveFn(error))
            return false;
        if (readFile(path) != bytes) {
            *error = text("game_settings.verify_failed",
                          {{QStringLiteral("path"), QDir::toNativeSeparators(path)}});
            return false;
        }
        return true;
    };
    if (attributes_->serialize() != savedAttributes_->serialize()) {
        if (!write(profile_.attributes, attributes_->serialize(), attributeBackups,
                   [this](QString *e) { return attributes_->save(profile_.attributes, e); }))
            return false;
        savedAttributes_ = attributes_;
    }
    if (actionMaps_->serialize() != savedActionMaps_->serialize()) {
        if (!write(profile_.actionMaps, actionMaps_->serialize(), actionBackups,
                   [this](QString *e) { return actionMaps_->save(profile_.actionMaps, e); }))
            return false;
        savedActionMaps_ = actionMaps_;
    }
    return true;
}

void GameSettingsController::save()
{
    if (!attributes_ || !actionMaps_ || pendingCount_ == 0)
        return;
    refreshGameRunning();
    if (gameRunning_) {
        app().prompts()->warning(text("game_settings.running_title"), text("game_settings.running_body"));
        return;
    }
    QString error;
    const bool ok = writeFiles(&error);
    edited();
    if (!ok) {
        app().prompts()->error(text("game_settings.save_failed_title"), error);
        return;
    }
    qCInfo(lcApp).noquote() << "Saved game settings to" << QDir::toNativeSeparators(profile_.dir);
    app().setStatus(text("game_settings.saved"));
}

void GameSettingsController::openProfileFolder()
{
    if (!profile_.dir.isEmpty())
        app().openFolder(QFileInfo(profile_.dir).isDir() ? profile_.dir
                                                         : QFileInfo(profile_.dir).absolutePath());
}

QVariantList GameSettingsController::backups() const
{
    QVariantList out;
    const QLocale locale;
    for (const auto &[prefix, file] : {std::pair{kAttributesBackup, QStringLiteral("attributes.xml")},
                                       std::pair{kActionMapsBackup, QStringLiteral("actionmaps.xml")}}) {
        const core::GameFileBackups backups(app().paths().backupsDir(), prefix);
        for (const QFileInfo &info : backups.list())
            out << QVariantMap{
                {QStringLiteral("path"), info.absoluteFilePath()},
                {QStringLiteral("file"), file},
                {QStringLiteral("date"), locale.toString(info.lastModified(), QLocale::ShortFormat)}};
    }
    return out;
}

void GameSettingsController::restoreBackup(const QString &path)
{
    refreshGameRunning();
    if (gameRunning_) {
        app().prompts()->warning(text("game_settings.running_title"), text("game_settings.running_body"));
        return;
    }
    const QString name = QFileInfo(path).fileName();
    const QString target = name.startsWith(kAttributesBackup)   ? profile_.attributes
                           : name.startsWith(kActionMapsBackup) ? profile_.actionMaps
                                                                : QString();
    if (target.isEmpty())
        return;
    const core::GameFileBackups backups(app().paths().backupsDir(), name.startsWith(kAttributesBackup)
                                                                        ? kAttributesBackup
                                                                        : kActionMapsBackup);
    QString error;
    if (QFileInfo::exists(target) && backups.backup(target, &error).isEmpty()) {
        app().prompts()->error(text("game_settings.save_failed_title"), error);
        return;
    }
    error = core::GameFileBackups::restore(path, target);
    if (!error.isEmpty()) {
        app().prompts()->error(text("game_settings.save_failed_title"), error);
        return;
    }
    app().setStatus(text("game_settings.restored"));
    reload();
}

QVariantList GameSettingsController::bindDevices() const
{
    QVariantList out;
    out << QVariantMap{{QStringLiteral("id"), QStringLiteral("keyboard")},
                       {QStringLiteral("title"), text("game_settings.devices.keyboard")}}
        << QVariantMap{{QStringLiteral("id"), QStringLiteral("mouse")},
                       {QStringLiteral("title"), text("game_settings.devices.mouse")}}
        << QVariantMap{{QStringLiteral("id"), QStringLiteral("gamepad")},
                       {QStringLiteral("title"), text("game_settings.devices.gamepad")}};
    int joysticks = 1;
    QHash<int, QString> products;
    if (actionMaps_) {
        for (const DeviceInfo &d : actionMaps_->devices())
            if (d.type == u"joystick" && !d.product.trimmed().isEmpty()) {
                joysticks = std::max(joysticks, d.instance);
                products.insert(d.instance, d.product.trimmed());
            }
        for (const BoundAction &b : actionMaps_->rebinds())
            if (b.device == Device::Joystick)
                joysticks = std::max(joysticks, b.instance);
    }
    for (int i = 1; i <= joysticks; ++i) {
        QString title = text("game_settings.devices.joystick", {{QStringLiteral("n"), i}});
        if (products.contains(i))
            title += QStringLiteral(" — ") +
                     products.value(i).section(QStringLiteral("  {"), 0, 0).section(u'{', 0, 0).trimmed();
        out << QVariantMap{{QStringLiteral("id"), QStringLiteral("joystick%1").arg(i)},
                           {QStringLiteral("title"), title}};
    }
    return out;
}

void GameSettingsController::setBindDevice(const QString &device)
{
    if (bindDevice_ == device)
        return;
    bindDevice_ = device;
    emit filtersChanged();
    refreshRows();
}

QStringList GameSettingsController::bindCategories() const
{
    QStringList out;
    if (!loaded_)
        return out;
    for (const ActionMapDef &map : loaded_->bindings.maps)
        if (!out.contains(map.label))
            out << map.label;
    return out;
}

void GameSettingsController::setBindCategory(const QString &category)
{
    if (bindCategory_ == category)
        return;
    bindCategory_ = category;
    emit filtersChanged();
    refreshRows();
}

GameSettingsController::BindTarget GameSettingsController::bindTarget() const
{
    if (bindDevice_.startsWith(QLatin1StringView("joystick")))
        return {Device::Joystick, std::max(1, bindDevice_.mid(8).toInt())};
    return {deviceFromId(bindDevice_).value_or(Device::Keyboard), 1};
}

QStringList GameSettingsController::inputChoices() const
{
    return loaded_ ? loaded_->keys.inputs(bindTarget().device) : QStringList();
}

QStringList GameSettingsController::activationModes() const
{
    return loaded_ ? loaded_->bindings.activationModes : QStringList();
}

QString GameSettingsController::bindingText(Device device, const QString &input, const QString &mode,
                                            int multiTap, const QString &defaultMode) const
{
    if (input.trimmed().isEmpty())
        return {};
    QString out = loaded_->keys.describe(device, input);
    if (multiTap > 1)
        out += QStringLiteral("  ×%1").arg(multiTap);
    if (!mode.isEmpty() && mode != defaultMode)
        out += QStringLiteral("  (") + labelFromName(mode).toLower() + u')';
    return out;
}

GameSettingsController::Effective GameSettingsController::effectiveBinding(const ActionDef &action) const
{
    const BindTarget target = bindTarget();
    Effective e;
    if (target.instance == 1)
        for (const DefaultBind &d : action.defaults)
            if (d.device == target.device) {
                e.defaultInput = d.input;
                e.defaultMode = d.activationMode;
                e.defaultMultiTap = d.multiTap;
                break;
            }
    if (const auto r = actionMaps_->rebind(action.map, action.name, target.device, target.instance)) {
        e.input = r->input;
        e.mode = r->activationMode;
        e.multiTap = r->multiTap;
        e.modified = true;
    } else {
        e.input = e.defaultInput;
        e.mode = e.defaultMode;
        e.multiTap = e.defaultMultiTap;
    }
    return e;
}

QString GameSettingsController::usageKey(const ActionDef &action, const Effective &e) const
{
    return QStringList{action.map, e.input, QString::number(std::max(1, e.multiTap)),
                       e.mode.isEmpty() ? action.activationMode : e.mode}
        .join(u'|');
}

void GameSettingsController::refreshBindUsage()
{
    bindUsage_.clear();
    if (!loaded_ || !actionMaps_ || tab_ != u"keybinds")
        return;
    for (const ActionMapDef &map : loaded_->bindings.maps)
        for (const ActionDef &action : map.actions) {
            const Effective e = effectiveBinding(action);
            if (!e.input.isEmpty())
                bindUsage_[usageKey(action, e)] << action.label;
        }
}

QVariantMap GameSettingsController::bindRow(const ActionMapDef &map, const ActionDef &action) const
{
    const Device device = bindTarget().device;
    const Effective e = effectiveBinding(action);
    QStringList others = e.input.isEmpty() ? QStringList() : bindUsage_.value(usageKey(action, e));
    others.removeOne(action.label);
    return {
        {QStringLiteral("map"), map.name},
        {QStringLiteral("action"), action.name},
        {QStringLiteral("label"), action.label},
        {QStringLiteral("mapLabel"), map.label},
        {QStringLiteral("description"), action.description},
        {QStringLiteral("binding"), bindingText(device, e.input, e.mode, e.multiTap, action.activationMode)},
        {QStringLiteral("input"), e.input},
        {QStringLiteral("activationMode"), e.mode.isEmpty() ? action.activationMode : e.mode},
        {QStringLiteral("multiTap"), std::max(1, e.multiTap)},
        {QStringLiteral("defaultText"),
         bindingText(device, e.defaultInput, e.defaultMode, e.defaultMultiTap, action.activationMode)},
        {QStringLiteral("modified"), e.modified},
        {QStringLiteral("conflicts"), others.join(QStringLiteral(", "))},
    };
}

void GameSettingsController::refreshBindRows()
{
    bindRows_.clear();
    if (!loaded_ || !actionMaps_ || tab_ != u"keybinds")
        return;
    for (const ActionMapDef &map : loaded_->bindings.maps) {
        if (!bindCategory_.isEmpty() && map.label != bindCategory_)
            continue;
        for (const ActionDef &action : map.actions) {
            const QVariantMap row = bindRow(map, action);
            if (!matches(search_, {action.label, map.label, action.name,
                                   row.value(QStringLiteral("binding")).toString(),
                                   row.value(QStringLiteral("input")).toString()}))
                continue;
            if (modifiedOnly_ && !row.value(QStringLiteral("modified")).toBool())
                continue;
            bindRows_ << row;
        }
    }
}

QVariantMap GameSettingsController::settingState(const QString &name, int) const
{
    if (!loaded_)
        return {};
    const int index = loaded_->settings.find(name);
    return index < 0 ? QVariantMap() : settingRow(loaded_->settings.settings[std::size_t(index)]);
}

QVariantMap GameSettingsController::bindState(const QString &map, const QString &action, int) const
{
    if (!loaded_ || !actionMaps_)
        return {};
    for (const ActionMapDef &m : loaded_->bindings.maps)
        if (m.name == map)
            for (const ActionDef &a : m.actions)
                if (a.name == action)
                    return bindRow(m, a);
    return {};
}

void GameSettingsController::setBinding(const QString &map, const QString &action, const QString &input,
                                        const QString &activationMode, int multiTap)
{
    if (!loaded_ || !actionMaps_)
        return;
    const BindTarget target = bindTarget();
    for (const ActionMapDef &m : loaded_->bindings.maps) {
        if (m.name != map)
            continue;
        for (const ActionDef &a : m.actions) {
            if (a.name != action)
                continue;
            if (target.instance == 1)
                for (const DefaultBind &d : a.defaults)
                    if (d.device == target.device && d.input == input.trimmed() &&
                        (activationMode.isEmpty() || activationMode == d.activationMode) &&
                        std::max(1, multiTap) == std::max(1, d.multiTap)) {
                        resetBinding(map, action);
                        return;
                    }
        }
    }
    const QString mode = activationMode;
    actionMaps_->setRebind(map, action, target.device, target.instance, {input.trimmed(), mode, multiTap});
    edited();
}

void GameSettingsController::unbind(const QString &map, const QString &action)
{
    if (!actionMaps_)
        return;
    const BindTarget target = bindTarget();
    actionMaps_->setRebind(map, action, target.device, target.instance, {});
    edited();
}

void GameSettingsController::resetBinding(const QString &map, const QString &action)
{
    if (!actionMaps_)
        return;
    const BindTarget target = bindTarget();
    actionMaps_->removeRebind(map, action, target.device, target.instance);
    edited();
}

void GameSettingsController::resetDeviceBindings()
{
    if (!actionMaps_)
        return;
    const BindTarget target = bindTarget();
    actionMaps_->removeRebinds(target.device, target.instance);
    edited();
}

QString GameSettingsController::keyName(int key, int nativeScanCode, int modifiers) const
{
    return keyNameFromQt(key, static_cast<quint32>(nativeScanCode), (modifiers & Qt::KeypadModifier) != 0);
}

QString GameSettingsController::mouseName(int button) const
{
    return mouseButtonName(button);
}

QString GameSettingsController::describeInput(const QString &input) const
{
    return loaded_ ? loaded_->keys.describe(bindTarget().device, input) : input;
}

std::vector<GameSettingsController::OptionTarget> GameSettingsController::optionTargets() const
{
    std::vector<OptionTarget> out;
    out.push_back({QStringLiteral("keyboard"), 1, {}, text("game_settings.devices.mouse_keyboard")});
    out.push_back({QStringLiteral("gamepad"), 1, {}, text("game_settings.devices.gamepad")});
    std::vector<OptionTarget> joysticks;
    if (actionMaps_)
        for (const DeviceInfo &d : actionMaps_->devices())
            if (d.type == u"joystick" && !d.product.trimmed().isEmpty())
                joysticks.push_back(
                    {d.type, d.instance, d.product,
                     text("game_settings.devices.joystick", {{QStringLiteral("n"), d.instance}}) +
                         QStringLiteral(" — ") + d.product.section(u'{', 0, 0).trimmed()});
    if (joysticks.empty())
        joysticks.push_back({QStringLiteral("joystick"),
                             1,
                             {},
                             text("game_settings.devices.joystick", {{QStringLiteral("n"), 1}})});
    std::sort(joysticks.begin(), joysticks.end(),
              [](const OptionTarget &a, const OptionTarget &b) { return a.instance < b.instance; });
    out.insert(out.end(), joysticks.begin(), joysticks.end());
    return out;
}

QVariantList GameSettingsController::optionDevices() const
{
    QVariantList out;
    for (const OptionTarget &t : optionTargets())
        out << QVariantMap{{QStringLiteral("title"), t.label},
                           {QStringLiteral("type"), t.type},
                           {QStringLiteral("instance"), t.instance}};
    return out;
}

void GameSettingsController::setOptionDevice(int index)
{
    if (optionDevice_ == index)
        return;
    optionDevice_ = index;
    emit filtersChanged();
    refreshRows();
}

QVariantMap GameSettingsController::optionRow(const OptionTarget &target, const OptionTree &tree,
                                              int index) const
{
    const OptionNode &node = tree.nodes[std::size_t(index)];
    const auto invert =
        actionMaps_->option(target.type, target.instance, node.name, QStringLiteral("invert"));
    const auto exponent =
        actionMaps_->option(target.type, target.instance, node.name, QStringLiteral("exponent"));
    const bool hasCurve = actionMaps_->curve(target.type, target.instance, node.name).has_value();
    const auto defaultExponent = tree.inheritedExponent(index);
    return {
        {QStringLiteral("group"), node.name},
        {QStringLiteral("label"), node.label},
        {QStringLiteral("depth"), node.depth},
        {QStringLiteral("showInvert"), node.showInvert == 1},
        {QStringLiteral("showCurve"), node.showCurve == 1},
        {QStringLiteral("inverted"),
         invert ? invert->toInt() != 0 : tree.inheritedInvert(index).value_or(false)},
        {QStringLiteral("exponent"), exponent.value_or(QString())},
        {QStringLiteral("defaultExponent"),
         defaultExponent ? formatNumber(*defaultExponent) : QStringLiteral("1")},
        {QStringLiteral("hasCurve"), hasCurve || !node.curve.empty()},
        {QStringLiteral("invertCvar"), node.invertCvar},
        {QStringLiteral("modified"), invert.has_value() || exponent.has_value() || hasCurve},
    };
}

QVariantMap GameSettingsController::axisRow(const OptionTarget &target, const QString &axis) const
{
    return {
        {QStringLiteral("input"), axis},
        {QStringLiteral("label"), loaded_->keys.label(Device::Joystick, axis)},
        {QStringLiteral("deadzone"),
         actionMaps_->deviceOption(target.product, axis, QStringLiteral("deadzone")).value_or(QString())},
        {QStringLiteral("saturation"),
         actionMaps_->deviceOption(target.product, axis, QStringLiteral("saturation")).value_or(QString())},
    };
}

void GameSettingsController::refreshOptionRows()
{
    optionRows_.clear();
    axisRows_.clear();
    if (!loaded_ || !actionMaps_ || tab_ != u"controls")
        return;
    const std::vector<OptionTarget> targets = optionTargets();
    if (optionDevice_ < 0 || optionDevice_ >= static_cast<int>(targets.size()))
        return;
    const OptionTarget &target = targets[std::size_t(optionDevice_)];
    const OptionTree *tree = loaded_->bindings.tree(target.type);
    if (!tree)
        return;
    for (std::size_t i = 0; i < tree->nodes.size(); ++i) {
        const OptionNode &node = tree->nodes[i];
        if (!matches(search_, {node.label, node.name}))
            continue;
        const QVariantMap row = optionRow(target, *tree, static_cast<int>(i));
        if (modifiedOnly_ && !row.value(QStringLiteral("modified")).toBool())
            continue;
        optionRows_ << row;
    }
    if (target.type == u"joystick" && !target.product.isEmpty())
        for (const QString &axis : kAxes)
            axisRows_ << axisRow(target, axis);
}

QVariantMap GameSettingsController::optionState(const QString &group, int) const
{
    const std::vector<OptionTarget> targets = optionTargets();
    if (!loaded_ || !actionMaps_ || optionDevice_ < 0 || optionDevice_ >= static_cast<int>(targets.size()))
        return {};
    const OptionTarget &target = targets[std::size_t(optionDevice_)];
    const OptionTree *tree = loaded_->bindings.tree(target.type);
    const int index = tree ? tree->find(group) : -1;
    return index < 0 ? QVariantMap() : optionRow(target, *tree, index);
}

QVariantMap GameSettingsController::axisState(const QString &input, int) const
{
    const std::vector<OptionTarget> targets = optionTargets();
    if (!loaded_ || !actionMaps_ || optionDevice_ < 0 || optionDevice_ >= static_cast<int>(targets.size()))
        return {};
    return axisRow(targets[std::size_t(optionDevice_)], input);
}

void GameSettingsController::setInvert(const QString &group, bool inverted)
{
    const std::vector<OptionTarget> targets = optionTargets();
    if (!actionMaps_ || optionDevice_ >= static_cast<int>(targets.size()))
        return;
    const OptionTarget &t = targets[std::size_t(optionDevice_)];
    actionMaps_->setOption(t.type, t.instance, group, QStringLiteral("invert"),
                           inverted ? QStringLiteral("1") : QStringLiteral("0"));
    edited();
}

void GameSettingsController::setExponent(const QString &group, const QString &value)
{
    const std::vector<OptionTarget> targets = optionTargets();
    if (!actionMaps_ || optionDevice_ >= static_cast<int>(targets.size()))
        return;
    const OptionTarget &t = targets[std::size_t(optionDevice_)];
    bool ok = false;
    const double number = value.toDouble(&ok);
    if (value.trimmed().isEmpty())
        actionMaps_->removeOption(t.type, t.instance, group, QStringLiteral("exponent"));
    else if (ok && number > 0.0)
        actionMaps_->setOption(t.type, t.instance, group, QStringLiteral("exponent"), formatNumber(number));
    else
        return;
    edited();
}

void GameSettingsController::resetOption(const QString &group)
{
    const std::vector<OptionTarget> targets = optionTargets();
    if (!actionMaps_ || optionDevice_ >= static_cast<int>(targets.size()))
        return;
    const OptionTarget &t = targets[std::size_t(optionDevice_)];
    actionMaps_->removeOption(t.type, t.instance, group, QStringLiteral("invert"));
    actionMaps_->removeOption(t.type, t.instance, group, QStringLiteral("exponent"));
    actionMaps_->removeCurve(t.type, t.instance, group);
    edited();
}

void GameSettingsController::setAxisOption(const QString &input, const QString &attribute,
                                           const QString &value)
{
    const std::vector<OptionTarget> targets = optionTargets();
    if (!actionMaps_ || optionDevice_ >= static_cast<int>(targets.size()))
        return;
    const OptionTarget &t = targets[std::size_t(optionDevice_)];
    if (t.product.isEmpty())
        return;
    bool ok = false;
    const double number = value.toDouble(&ok);
    if (value.trimmed().isEmpty())
        actionMaps_->removeDeviceOption(t.product, input, attribute);
    else if (ok && number >= 0.0 && number <= 1.0)
        actionMaps_->setDeviceOption(t.product, input, attribute, formatNumber(number));
    else
        return;
    edited();
}

void GameSettingsController::resetAxisOption(const QString &input, const QString &attribute)
{
    setAxisOption(input, attribute, {});
}
