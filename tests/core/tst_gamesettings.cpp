#include "core/gamesettings/ActionMapsFile.h"
#include "core/gamesettings/Attributes.h"
#include "core/gamesettings/Bindings.h"
#include "core/gamesettings/GameProfile.h"
#include "core/gamesettings/SettingsCatalog.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTemporaryDir>
#include <QTest>

using namespace core::gamesettings;

namespace {

const QByteArray kDefaults = R"(<Attributes Version="35">
  <Attr name="Resolution" value="-1" platform="pc" confirmation="1" />
  <Attr name="SprintToggle" cvar="cl_sprinttoggle" value="0" />
  <Attr name="Height" cvar="r_height" value="720" platform="pc" confirmation="1" />
  <Attr name="Height" cvar="r_height" value="1080" platform="ps5|xbox_x" confirmation="1" />
  <Attr name="SysSpec" cvar="sys_spec" value="9" platform="ps5" />
  <Attr name="SysSpec" cvar="sys_spec" value="-1" platform="pc" />
  <Attr name="Gamma" cvar="r_Gamma" value="1.0025" preview="1" />
  <Attr name="AudioMasterVolume" cvar="s_MixLevelMaster" value="1.0" preview="1" />
  <Attr name="IFCS_Setting_CoupledEnabled" cvar="v_coupled" value="1" />
  <Attr name="HmdIPDScale" cvar="r_StereoStrength" value="1" />
  <Attr name="MSAAMode" value="smaa_2tx" platform="pc" />
</Attributes>
)";

const QByteArray kDefinitions = R"({
  "otherLabel": "other.label",
  "tabs": [ { "id": "gameplay", "label": "@tab_gameplay" }, { "id": "graphics", "label": "=Graphics" },
            { "id": "audio", "label": "tabs.audio" }, { "id": "comms", "label": "=Comms" } ],
  "fallback": [ { "prefix": "Hmd", "tab": "comms" }, { "prefix": "MSAA", "tab": "graphics" },
                { "prefix": "", "tab": "gameplay" } ],
  "choiceSets": { "quality": [ { "value": "-1", "label": "=Auto" }, { "value": "1", "label": "@low" } ] },
  "sections": [
    { "id": "gameplay.general", "tab": "gameplay", "label": "=General", "settings": [
      { "name": "SprintToggle", "label": "@pause_OptionsSprintToggle", "type": "toggle" },
      { "name": "IFCS_Setting_CoupledEnabled", "label": "@decoupled", "type": "toggle", "invert": true },
      { "name": "DoesNotExist", "label": "=Gone", "type": "toggle" }
    ] },
    { "id": "graphics.quality", "tab": "graphics", "label": "=Quality", "settings": [
      { "name": "SysSpec", "label": "@missing_key", "type": "choice", "choices": "quality" },
      { "name": "Gamma", "label": "=Gamma", "type": "number" },
      { "name": "Height", "label": "=Height", "type": "number", "restart": true }
    ] },
    { "id": "audio.volume", "tab": "audio", "label": "=Volume", "settings": [
      { "name": "AudioMasterVolume", "label": "=Master", "type": "slider", "min": 0, "max": 1, "step": 0.01,
        "scale": 100, "unit": "=%" }
    ] }
  ]
})";

const QByteArray kProfile = R"(<profile version="1" optionsVersion="2" rebindVersion="2">
  <ActivationModes>
    <ActivationMode name="tap" onPress="0" />
    <ActivationMode name="press" onPress="1" />
  </ActivationModes>
  <optiontree type="keyboard" name="root" UISensitivityMin="0.01" UISensitivityMax="6.25">
    <optiongroup name="master" UILabel="@ui_master" UIShowInvert="0">
      <optiongroup name="flight" UILabel="@ui_flight" UIShowInvert="-1" invert="1" exponent="2.5">
        <nonlinearity_curve>
          <point in="0.5" out="0.25" />
        </nonlinearity_curve>
        <optiongroup name="flight_move_pitch" UILabel="@ui_pitch" UIShowCurve="1" UIShowInvert="1" />
      </optiongroup>
    </optiongroup>
  </optiontree>
  <actionmap name="spaceship_general" version="2" UILabel="@ui_general" UICategory="@ui_flight_cat">
    <action name="v_eject" activationMode="press" keyboard="ralt+l" gamepad=" " joystick=" " UILabel="@ui_eject" UIDescription="@ui_eject_desc" />
    <action name="v_ads" activationMode="tap" keyboard="mouse2" UILabel="@ui_ads">
      <gamepad activationMode="hold" input="triggerl_btn" />
    </action>
    <action name="ui_up" keyboard="up" UILabel="@ui_up">
      <gamepad>
        <inputdata input="dpad_up" />
        <inputdata input="thumbr_up" />
      </gamepad>
    </action>
    <action name="hidden_action" keyboard="h" />
  </actionmap>
  <actionmap name="debug" version="1">
    <action name="debug_thing" keyboard="f12" UILabel="@ui_debug" />
  </actionmap>
</profile>
)";

const QByteArray kKeys = R"(<LocalizedKeys>
  <device name="keyboard">
    <Key name="ralt" localizationString="@key_ralt" />
    <Key name="l" localizationString="@key_l" />
    <Key name="mouse2" localizationString="@key_mouse2" />
  </device>
  <device name="mouse">
    <Key name="mouse2" localizationString="@key_mouse2" />
  </device>
  <device name="joystick1">
    <Key name="button3" localizationString="@key_js_button3" />
  </device>
</LocalizedKeys>
)";

const QByteArray kUserActionMaps =
    "<ActionMaps>\r\n"
    " <ActionProfiles version=\"1\" optionsVersion=\"2\" rebindVersion=\"2\" "
    "profileName=\"default\">\r\n"
    "  <options type=\"keyboard\" instance=\"1\" Product=\"Keyboard  {6F1D}\"/>\r\n"
    "  <options type=\"joystick\" instance=\"1\" Product=\" VKB Gladiator  {0200}\"/>\r\n"
    "  <modifiers />\r\n"
    "  <actionmap name=\"spaceship_general\">\r\n"
    "   <action name=\"v_eject\">\r\n"
    "    <rebind input=\"js1_button3\"/>\r\n"
    "   </action>\r\n"
    "  </actionmap>\r\n"
    " </ActionProfiles>\r\n"
    "</ActionMaps>\r\n";

TextLookup lookup()
{
    TextLookup text;
    text.game = [](const QString &key) -> QString {
        static const QHash<QString, QString> loc = {
            {QStringLiteral("tab_gameplay"), QStringLiteral("GAME SETTINGS")},
            {QStringLiteral("pause_OptionsSprintToggle"), QStringLiteral("Sprint Toggle")},
            {QStringLiteral("pause_OptionsSprintToggleDesc"), QStringLiteral("Hold or toggle sprint.")},
            {QStringLiteral("decoupled"), QStringLiteral("Decoupled On")},
            {QStringLiteral("low"), QStringLiteral("Low")},
            {QStringLiteral("ui_general"), QStringLiteral("Flight - General")},
            {QStringLiteral("ui_flight_cat"), QStringLiteral("FLIGHT")},
            {QStringLiteral("ui_eject"), QStringLiteral("Eject")},
            {QStringLiteral("ui_eject_desc"), QStringLiteral("Leave in a hurry.")},
            {QStringLiteral("ui_flight"), QStringLiteral("Flight")},
            {QStringLiteral("ui_pitch"), QStringLiteral("Pitch")},
            {QStringLiteral("key_ralt"), QStringLiteral("Right Alt")},
            {QStringLiteral("key_l"), QStringLiteral("L")},
            {QStringLiteral("key_mouse2"), QStringLiteral("Right Mouse Button")},
            {QStringLiteral("key_js_button3"), QStringLiteral("Button 3")},
        };
        return loc.value(key);
    };
    text.app = [](const QString &key) { return QStringLiteral("app:") + key; };
    return text;
}

SettingsCatalog catalog(const QStringList &extra = {})
{
    return buildSettingsCatalog(QJsonDocument::fromJson(kDefinitions).object(),
                                parseAttributeDefaults(kDefaults), extra, lookup());
}

const SettingDef &setting(const SettingsCatalog &c, const char *name)
{
    return c.settings[std::size_t(c.find(QString::fromLatin1(name)))];
}

}

class TestGameSettings : public QObject
{
    Q_OBJECT

private slots:
    void defaultsKeepPcEntriesOnce()
    {
        const AttributeDefaults defaults = parseAttributeDefaults(kDefaults);
        QCOMPARE(defaults.version, QStringLiteral("35"));
        QStringList names;
        for (const AttributeDef &d : defaults.attributes)
            names << d.name;
        QCOMPARE(names.count(QStringLiteral("Height")), 1);
        QCOMPARE(names.count(QStringLiteral("SysSpec")), 1);
        auto find = [&](const char *name) {
            return *std::find_if(defaults.attributes.begin(), defaults.attributes.end(),
                                 [name](const AttributeDef &d) { return d.name == QLatin1StringView(name); });
        };
        QCOMPARE(find("Height").defaultValue, QStringLiteral("720"));
        QCOMPARE(find("SysSpec").defaultValue, QStringLiteral("-1"));
        QVERIFY(find("Height").confirmation);
        QVERIFY(find("Gamma").preview);
        QCOMPARE(find("SprintToggle").cvar, QStringLiteral("cl_sprinttoggle"));
    }

    void formatsNumbersLikeTheGame()
    {
        QCOMPARE(formatNumber(1.0), QStringLiteral("1"));
        QCOMPARE(formatNumber(83.9863), QStringLiteral("83.9863"));
        QCOMPARE(formatNumber(0.029), QStringLiteral("0.029"));
        QCOMPARE(formatNumber(100000), QStringLiteral("100000"));
        QCOMPARE(formatNumber(-18), QStringLiteral("-18"));
        QCOMPARE(formatNumber(-0.0), QStringLiteral("0"));
        QVERIFY(sameValue(QStringLiteral("1.0"), QStringLiteral("1")));
        QVERIFY(sameValue(QStringLiteral("smaa_2tx"), QStringLiteral("smaa_2tx")));
        QVERIFY(!sameValue(QStringLiteral("0.5"), QStringLiteral("0.75")));
    }

    void attributesFileRoundTripsInGameFormat()
    {
        const QByteArray original = "<Attributes Version=\"35\">\r\n"
                                    " <Attr name=\"FOV\" value=\"83.9863\"/>\r\n"
                                    " <Attr name=\"ADSMouseSensitivity\" value=\"1\"/>\r\n"
                                    " <Attr name=\"Preset0\" value=\"\"/>\r\n"
                                    "</Attributes>\r\n";
        auto file = AttributesFile::parse(original);
        QVERIFY(file);
        QCOMPARE(file->version(), QStringLiteral("35"));
        QCOMPARE(file->value(QStringLiteral("FOV")).value_or(QString()), QStringLiteral("83.9863"));
        QVERIFY(!file->value(QStringLiteral("Missing")));
        file->set(QStringLiteral("AspectModifier"), QStringLiteral("a<b&\"c\""));
        QVERIFY(file->remove(QStringLiteral("FOV")));
        QVERIFY(!file->remove(QStringLiteral("FOV")));
        QCOMPARE(file->serialize(),
                 QByteArray("<Attributes Version=\"35\">\r\n"
                            " <Attr name=\"ADSMouseSensitivity\" value=\"1\"/>\r\n"
                            " <Attr name=\"AspectModifier\" value=\"a&lt;b&amp;&quot;c&quot;\"/>\r\n"
                            " <Attr name=\"Preset0\" value=\"\"/>\r\n"
                            "</Attributes>\r\n"));
        const auto again = AttributesFile::parse(file->serialize());
        QVERIFY(again);
        QCOMPARE(again->value(QStringLiteral("AspectModifier")).value_or(QString()),
                 QStringLiteral("a<b&\"c\""));
        QVERIFY(!AttributesFile::parse("<NotAttributes />"));
        QVERIFY(!AttributesFile::parse("<Attributes"));
    }

    void attributesSaveAndLoad()
    {
        QTemporaryDir dir;
        AttributesFile file;
        file.setVersion(QStringLiteral("35"));
        file.set(QStringLiteral("Width"), QStringLiteral("1920"));
        const QString path = dir.filePath(QStringLiteral("Profiles/default/attributes.xml"));
        QString error;
        QVERIFY2(file.save(path, &error), qPrintable(error));
        const auto loaded = AttributesFile::load(path);
        QVERIFY(loaded);
        QCOMPARE(loaded->serialize(), file.serialize());
    }

    void catalogPlacesCuratedSettings()
    {
        const SettingsCatalog c = catalog();
        QCOMPARE(c.tabs.size(), std::size_t(4));
        QCOMPARE(c.tabs[0].label, QStringLiteral("GAME SETTINGS"));
        QCOMPARE(c.tabs[1].label, QStringLiteral("Graphics"));
        QCOMPARE(c.tabs[2].label, QStringLiteral("app:tabs.audio"));
        QCOMPARE(c.find(QStringLiteral("DoesNotExist")), -1);

        const SettingDef &sprint = setting(c, "SprintToggle");
        QCOMPARE(sprint.label, QStringLiteral("Sprint Toggle"));
        QCOMPARE(sprint.description, QStringLiteral("Hold or toggle sprint."));
        QCOMPARE(sprint.kind, SettingKind::Toggle);
        QCOMPARE(sprint.section, QStringLiteral("gameplay.general"));
        QVERIFY(sprint.curated);

        const SettingDef &spec = setting(c, "SysSpec");
        QCOMPARE(spec.label, QStringLiteral("Sys Spec"));
        QCOMPARE(spec.choices.size(), 2);
        QCOMPARE(spec.choices[1].label, QStringLiteral("Low"));
        QCOMPARE(spec.defaultValue, QStringLiteral("-1"));

        const SettingDef &height = setting(c, "Height");
        QVERIFY(height.restart);
        QCOMPARE(height.defaultValue, QStringLiteral("720"));

        const SettingDef &volume = setting(c, "AudioMasterVolume");
        QCOMPARE(volume.kind, SettingKind::Slider);
        QCOMPARE(volume.scale, 100.0);
        QCOMPARE(volume.unit, QStringLiteral("%"));
    }

    void catalogPutsEverythingElseInOther()
    {
        const SettingsCatalog c = catalog({QStringLiteral("Preset0")});
        const SettingDef &ipd = setting(c, "HmdIPDScale");
        QCOMPARE(ipd.tab, QStringLiteral("comms"));
        QCOMPARE(ipd.section, QStringLiteral("comms.other"));
        QCOMPARE(ipd.kind, SettingKind::Number);
        QVERIFY(!ipd.curated);
        QCOMPARE(ipd.label, QStringLiteral("Hmd IPD Scale"));
        const SettingDef &msaa = setting(c, "MSAAMode");
        QCOMPARE(msaa.tab, QStringLiteral("graphics"));
        QCOMPARE(msaa.kind, SettingKind::Text);
        const SettingDef &resolution = setting(c, "Resolution");
        QCOMPARE(resolution.tab, QStringLiteral("gameplay"));
        const SettingDef &preset = setting(c, "Preset0");
        QVERIFY(!preset.inDefaults);
        QCOMPARE(preset.section, QStringLiteral("gameplay.other"));
        const auto other = std::find_if(c.sections.begin(), c.sections.end(),
                                        [](const SettingsSection &s) { return s.id == u"comms.other"; });
        QVERIFY(other != c.sections.end());
        QCOMPARE(other->label, QStringLiteral("app:other.label"));
        std::size_t placed = 0;
        for (const SettingsSection &s : c.sections)
            placed += s.settings.size();
        QCOMPARE(placed, c.settings.size());
    }

    void invertedTogglesFlipTheStoredValue()
    {
        const SettingsCatalog c = catalog();
        const SettingDef &coupled = setting(c, "IFCS_Setting_CoupledEnabled");
        QVERIFY(coupled.invert);
        QVERIFY(!toggleChecked(coupled, QStringLiteral("1")));
        QVERIFY(toggleChecked(coupled, QStringLiteral("0")));
        QCOMPARE(toggleValue(coupled, true), QStringLiteral("0"));
        const SettingDef &sprint = setting(c, "SprintToggle");
        QVERIFY(toggleChecked(sprint, QStringLiteral("1.0")));
        QCOMPARE(toggleValue(sprint, true), QStringLiteral("1"));
    }

    void labelsFromNames()
    {
        QCOMPARE(labelFromName(QStringLiteral("HeadtrackingSource")), QStringLiteral("Headtracking Source"));
        QCOMPARE(labelFromName(QStringLiteral("SysSpec_TextureQuality")),
                 QStringLiteral("Sys Spec Texture Quality"));
        QCOMPARE(labelFromName(QStringLiteral("VJoyMgvAngle")), QStringLiteral("V Joy Mgv Angle"));
    }

    void bundledDefinitionsParse()
    {
        QFile file(QStringLiteral(SC_SOURCE_DIR "/resources/gamesettings/settings.json"));
        QVERIFY(file.open(QIODevice::ReadOnly));
        QJsonParseError error{};
        const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
        QCOMPARE(error.error, QJsonParseError::NoError);
        const QJsonObject root = doc.object();
        QCOMPARE(root.value(QLatin1StringView("tabs")).toArray().size(), 6);
        QSet<QString> tabs;
        for (const QJsonValue &t : root.value(QLatin1StringView("tabs")).toArray())
            tabs.insert(t.toObject().value(QLatin1StringView("id")).toString());
        QSet<QString> names;
        for (const QJsonValue &s : root.value(QLatin1StringView("sections")).toArray()) {
            QVERIFY(tabs.contains(s.toObject().value(QLatin1StringView("tab")).toString()));
            for (const QJsonValue &e : s.toObject().value(QLatin1StringView("settings")).toArray()) {
                const QJsonObject entry = e.toObject();
                const QString name = entry.value(QLatin1StringView("name")).toString();
                QVERIFY2(!names.contains(name), qPrintable(name));
                names.insert(name);
                const QJsonValue choices = entry.value(QLatin1StringView("choices"));
                if (choices.isString())
                    QVERIFY2(
                        root.value(QLatin1StringView("choiceSets")).toObject().contains(choices.toString()),
                        qPrintable(name));
            }
        }
        QVERIFY(names.size() > 300);
    }

    void parsesDefaultProfile()
    {
        QString error;
        const BindingsCatalog b = parseDefaultProfile(kProfile, lookup(), &error);
        QVERIFY2(error.isEmpty(), qPrintable(error));
        QCOMPARE(b.activationModes, QStringList({QStringLiteral("tap"), QStringLiteral("press")}));
        QCOMPARE(b.maps.size(), std::size_t(1));
        const ActionMapDef &map = b.maps[0];
        QCOMPARE(map.label, QStringLiteral("Flight - General"));
        QCOMPARE(map.category, QStringLiteral("FLIGHT"));
        QCOMPARE(map.actions.size(), std::size_t(3));

        const ActionDef &eject = map.actions[0];
        QCOMPARE(eject.label, QStringLiteral("Eject"));
        QCOMPARE(eject.description, QStringLiteral("Leave in a hurry."));
        QCOMPARE(eject.defaults.size(), std::size_t(1));
        QCOMPARE(eject.defaults[0].device, Device::Keyboard);
        QCOMPARE(eject.defaults[0].input, QStringLiteral("ralt+l"));
        QCOMPARE(eject.defaults[0].activationMode, QStringLiteral("press"));

        const ActionDef &ads = map.actions[1];
        QCOMPARE(ads.defaults.size(), std::size_t(2));
        QCOMPARE(ads.defaults[0].device, Device::Mouse);
        QCOMPARE(ads.defaults[1].device, Device::Gamepad);
        QCOMPARE(ads.defaults[1].activationMode, QStringLiteral("hold"));

        const ActionDef &up = map.actions[2];
        QCOMPARE(up.label, QStringLiteral("ui up"));
        QCOMPARE(up.defaults.size(), std::size_t(3));
        QCOMPARE(up.defaults[2].input, QStringLiteral("thumbr_up"));
    }

    void parsesOptionTrees()
    {
        const BindingsCatalog b = parseDefaultProfile(kProfile, lookup());
        const OptionTree *tree = b.tree(QStringLiteral("keyboard"));
        QVERIFY(tree);
        QVERIFY(!b.tree(QStringLiteral("gamepad")));
        QCOMPARE(tree->sensitivityMax, 6.25);
        QCOMPARE(tree->nodes.size(), std::size_t(3));
        const int pitch = tree->find(QStringLiteral("flight_move_pitch"));
        QCOMPARE(pitch, 2);
        QCOMPARE(tree->nodes[2].depth, 2);
        QCOMPARE(tree->nodes[2].showInvert, 1);
        QCOMPARE(tree->nodes[1].curve.size(), std::size_t(1));
        QVERIFY(tree->inheritedInvert(pitch) == std::optional<bool>(true));
        QVERIFY(tree->inheritedExponent(pitch) == std::optional<double>(2.5));
        QVERIFY(!tree->inheritedInvert(0));
    }

    void describesInputs()
    {
        const KeyLabels keys = KeyLabels::parse(kKeys, lookup());
        QCOMPARE(keys.describe(Device::Keyboard, QStringLiteral("ralt+l")), QStringLiteral("Right Alt + L"));
        QCOMPARE(keys.describe(Device::Mouse, QStringLiteral("ralt+mouse2")),
                 QStringLiteral("Right Alt + Right Mouse Button"));
        QCOMPARE(keys.describe(Device::Joystick, QStringLiteral("button3")), QStringLiteral("Button 3"));
        QCOMPARE(keys.describe(Device::Joystick, QStringLiteral("hat1_up")), QStringLiteral("Hat1 up"));
        QCOMPARE(keys.describe(Device::Keyboard, QStringLiteral(" ")), QString());
        QCOMPARE(keys.inputs(Device::Keyboard), QStringList({QStringLiteral("ralt"), QStringLiteral("l")}));
    }

    void parsesPrefixedInputs()
    {
        auto parsed = parseInput(QStringLiteral("js2_button12"));
        QVERIFY(parsed);
        QCOMPARE(parsed->device, Device::Joystick);
        QCOMPARE(parsed->instance, 2);
        QCOMPARE(parsed->input, QStringLiteral("button12"));
        parsed = parseInput(QStringLiteral("kb1_ "));
        QVERIFY(parsed);
        QVERIFY(parsed->input.isEmpty());
        QCOMPARE(parseInput(QStringLiteral("mo1_lctrl+mouse2"))->device, Device::Mouse);
        QVERIFY(!parseInput(QStringLiteral("button3")));
        QCOMPARE(inputPrefix(Device::Gamepad, 1), QStringLiteral("gp1_"));
        QCOMPARE(blankInput(QString()), QStringLiteral(" "));
        QVERIFY(isMouseInput(QStringLiteral("lalt+mwheel_up")));
        QVERIFY(!isMouseInput(QStringLiteral("lalt+m")));
        QCOMPARE(defaultDevice(QStringLiteral("keyboard"), QStringLiteral("mouse1")), Device::Mouse);
    }

    void namesCapturedKeys()
    {
        QCOMPARE(keyNameFromQt(Qt::Key_K, 0, false), QStringLiteral("k"));
        QCOMPARE(keyNameFromQt(Qt::Key_7, 0, false), QStringLiteral("7"));
        QCOMPARE(keyNameFromQt(Qt::Key_7, 0, true), QStringLiteral("np_7"));
        QCOMPARE(keyNameFromQt(Qt::Key_Plus, 0, true), QStringLiteral("np_add"));
        QCOMPARE(keyNameFromQt(Qt::Key_F11, 0, false), QStringLiteral("f11"));
        QCOMPARE(keyNameFromQt(Qt::Key_PageDown, 0, false), QStringLiteral("pgdn"));
        QCOMPARE(keyNameFromQt(Qt::Key_BracketLeft, 0, false), QStringLiteral("lbracket"));
#ifdef Q_OS_WIN
        QCOMPARE(keyNameFromQt(Qt::Key_Shift, 0x36, false), QStringLiteral("rshift"));
        QCOMPARE(keyNameFromQt(Qt::Key_Control, 0x1D, false), QStringLiteral("lctrl"));
        QCOMPARE(keyNameFromQt(Qt::Key_Alt, 0x138, false), QStringLiteral("ralt"));
#else
        QCOMPARE(keyNameFromQt(Qt::Key_Shift, 62, false), QStringLiteral("rshift"));
        QCOMPARE(keyNameFromQt(Qt::Key_Control, 37, false), QStringLiteral("lctrl"));
        QCOMPARE(keyNameFromQt(Qt::Key_Alt, 108, false), QStringLiteral("ralt"));
#endif
        QCOMPARE(keyNameFromQt(Qt::Key_Shift, 0, false), QStringLiteral("lshift"));
        QCOMPARE(keyNameFromQt(Qt::Key_AltGr, 0, false), QStringLiteral("ralt"));
        QCOMPARE(keyNameFromQt(Qt::Key_Meta, 0, false), QString());
        QCOMPARE(mouseButtonName(Qt::RightButton), QStringLiteral("mouse2"));
        QCOMPARE(mouseButtonName(Qt::NoButton), QString());
        QVERIFY(isModifierName(QStringLiteral("rctrl")));
        QVERIFY(!isModifierName(QStringLiteral("k")));
    }

    void readsUserActionMaps()
    {
        auto file = ActionMapsFile::parse(kUserActionMaps);
        QVERIFY(file);
        const auto rebind =
            file->rebind(QStringLiteral("spaceship_general"), QStringLiteral("v_eject"), Device::Joystick, 1);
        QVERIFY(rebind);
        QCOMPARE(rebind->input, QStringLiteral("button3"));
        QVERIFY(!file->rebind(QStringLiteral("spaceship_general"), QStringLiteral("v_eject"),
                              Device::Keyboard, 1));
        const std::vector<DeviceInfo> devices = file->devices();
        QCOMPARE(devices.size(), std::size_t(2));
        QCOMPARE(devices[1].product, QStringLiteral(" VKB Gladiator  {0200}"));
        const auto again = ActionMapsFile::parse(file->serialize());
        QVERIFY(again);
        QCOMPARE(again->serialize(), file->serialize());
        QVERIFY(file->serialize().contains("<rebind input=\"js1_button3\" />"));
        QVERIFY(!ActionMapsFile::parse("<Other />"));
    }

    void editsRebinds()
    {
        auto file = ActionMapsFile::parse(kUserActionMaps);
        QVERIFY(file);
        file->setRebind(QStringLiteral("player"), QStringLiteral("jump"), Device::Keyboard, 1,
                        {QStringLiteral("lalt+space"), QStringLiteral("press"), 2});
        auto r = file->rebind(QStringLiteral("player"), QStringLiteral("jump"), Device::Keyboard, 1);
        QVERIFY(r);
        QCOMPARE(r->input, QStringLiteral("lalt+space"));
        QCOMPARE(r->activationMode, QStringLiteral("press"));
        QCOMPARE(r->multiTap, 2);
        QVERIFY(file->serialize().contains(
            "<rebind input=\"kb1_lalt+space\" activationMode=\"press\" multiTap=\"2\" />"));

        file->setRebind(QStringLiteral("player"), QStringLiteral("jump"), Device::Keyboard, 1, {});
        r = file->rebind(QStringLiteral("player"), QStringLiteral("jump"), Device::Keyboard, 1);
        QVERIFY(r);
        QVERIFY(r->input.isEmpty());
        QVERIFY(file->serialize().contains("<rebind input=\"kb1_ \" />"));

        QCOMPARE(file->rebinds().size(), std::size_t(2));
        QVERIFY(file->removeRebind(QStringLiteral("player"), QStringLiteral("jump"), Device::Keyboard, 1));
        QVERIFY(!file->serialize().contains("name=\"player\""));
        QCOMPARE(file->removeRebinds(Device::Joystick, 1), 1);
        QVERIFY(!file->serialize().contains("<actionmap"));
        QVERIFY(file->serialize().contains("<modifiers />"));
    }

    void editsOptionsAndDeadzones()
    {
        ActionMapsFile file = ActionMapsFile::empty();
        file.setOption(QStringLiteral("joystick"), 2, QStringLiteral("flight_move_pitch"),
                       QStringLiteral("invert"), QStringLiteral("1"));
        QCOMPARE(file.option(QStringLiteral("joystick"), 2, QStringLiteral("flight_move_pitch"),
                             QStringLiteral("invert"))
                     .value_or(QString()),
                 QStringLiteral("1"));
        QVERIFY(!file.option(QStringLiteral("joystick"), 1, QStringLiteral("flight_move_pitch"),
                             QStringLiteral("invert")));
        const QByteArray text = file.serialize();
        QVERIFY(text.indexOf("<options type=\"joystick\" instance=\"2\">") < text.indexOf("<modifiers />"));

        file.setCurve(QStringLiteral("joystick"), 2, QStringLiteral("flight_move_pitch"), {{0.5, 0.25}});
        const auto curve = file.curve(QStringLiteral("joystick"), 2, QStringLiteral("flight_move_pitch"));
        QVERIFY(curve);
        QCOMPARE(curve->size(), std::size_t(1));
        QVERIFY(file.removeCurve(QStringLiteral("joystick"), 2, QStringLiteral("flight_move_pitch")));
        QVERIFY(file.removeOption(QStringLiteral("joystick"), 2, QStringLiteral("flight_move_pitch"),
                                  QStringLiteral("invert")));
        QVERIFY(!file.serialize().contains("flight_move_pitch"));

        const QString product = QStringLiteral(" VKB Gladiator  {0200}");
        file.setDeviceOption(product, QStringLiteral("x"), QStringLiteral("deadzone"),
                             QStringLiteral("0.05"));
        QCOMPARE(
            file.deviceOption(product, QStringLiteral("x"), QStringLiteral("deadzone")).value_or(QString()),
            QStringLiteral("0.05"));
        QVERIFY(file.serialize().contains("<option input=\"x\" deadzone=\"0.05\" />"));
        QVERIFY(file.removeDeviceOption(product, QStringLiteral("x"), QStringLiteral("deadzone")));
        QVERIFY(!file.serialize().contains("deviceoptions"));
    }

    void copiesAreIndependent()
    {
        auto original = ActionMapsFile::parse(kUserActionMaps);
        QVERIFY(original);
        ActionMapsFile copy = *original;
        copy.removeRebinds(Device::Joystick, 1);
        QCOMPARE(original->rebinds().size(), std::size_t(1));
        QCOMPARE(copy.rebinds().size(), std::size_t(0));
        copy = *original;
        QCOMPARE(copy.serialize(), original->serialize());
    }

    void savesWithCrlf()
    {
        QTemporaryDir dir;
        const ActionMapsFile file = ActionMapsFile::empty();
        const QString path = dir.filePath(QStringLiteral("actionmaps.xml"));
        QVERIFY(file.save(path));
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray bytes = f.readAll();
        QVERIFY(bytes.contains("\r\n"));
        QVERIFY(!bytes.contains("\r\r"));
        QCOMPARE(bytes.count('\n'), bytes.count("\r\n"));
        QVERIFY(bytes.startsWith("<ActionMaps>"));
    }

    void realGameData()
    {
        const QString config = qEnvironmentVariable("SCX_GAME_CONFIG_DIR");
        const QString profile = qEnvironmentVariable("SCX_GAME_PROFILE_DIR");
        if (config.isEmpty())
            QSKIP("set SCX_GAME_CONFIG_DIR (and optionally SCX_GAME_PROFILE_DIR) to check real game files");
        auto read = [](const QString &path) {
            QFile f(path);
            return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
        };
        const AttributeDefaults defaults =
            parseAttributeDefaults(read(config + QStringLiteral("/attributes.xml")));
        QVERIFY(defaults.attributes.size() > 300);
        const QJsonObject definitions =
            QJsonDocument::fromJson(
                read(QStringLiteral(SC_SOURCE_DIR "/resources/gamesettings/settings.json")))
                .object();
        const SettingsCatalog c = buildSettingsCatalog(definitions, defaults, {}, lookup());
        QCOMPARE(c.settings.size(), defaults.attributes.size());
        int curated = 0;
        for (const SettingDef &def : c.settings)
            curated += def.curated ? 1 : 0;
        int listed = 0;
        for (const QJsonValue &s : definitions.value(QLatin1StringView("sections")).toArray())
            listed += static_cast<int>(s.toObject().value(QLatin1StringView("settings")).toArray().size());
        QCOMPARE(curated, listed);

        QString error;
        const BindingsCatalog b =
            parseDefaultProfile(read(config + QStringLiteral("/defaultProfile.xml")), lookup(), &error);
        QVERIFY2(error.isEmpty(), qPrintable(error));
        QVERIFY(b.maps.size() > 30);
        QCOMPARE(b.optionTrees.size(), std::size_t(3));
        const KeyLabels keys =
            KeyLabels::parse(read(config + QStringLiteral("/keybinding_localization.xml")), lookup());
        QVERIFY(keys.inputs(Device::Keyboard).contains(QStringLiteral("lalt")));

        if (profile.isEmpty())
            return;
        const QByteArray attributes = read(profile + QStringLiteral("/attributes.xml"));
        if (!attributes.isEmpty()) {
            const auto file = AttributesFile::parse(attributes);
            QVERIFY(file);
            QCOMPARE(file->serialize(), attributes);
        }
        const QByteArray actionMaps = read(profile + QStringLiteral("/actionmaps.xml"));
        if (!actionMaps.isEmpty()) {
            const auto file = ActionMapsFile::parse(actionMaps);
            QVERIFY(file);
            const auto again = ActionMapsFile::parse(file->serialize());
            QVERIFY(again);
            QCOMPARE(again->serialize(), file->serialize());
            QCOMPARE(again->rebinds().size(), file->rebinds().size());
        }
    }

    void resolvesProfileFoldersAnyCase()
    {
        QTemporaryDir dir;
        QVERIFY(QDir(dir.path()).mkpath(QStringLiteral("LIVE/USER/Client/0/Profiles/default")));
        QFile f(dir.filePath(QStringLiteral("LIVE/USER/Client/0/Profiles/default/attributes.xml")));
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.close();
        const ProfilePaths paths = profilePaths(dir.filePath(QStringLiteral("LIVE")));
        QCOMPARE(paths.dir, dir.filePath(QStringLiteral("LIVE/USER/Client/0/Profiles/default")));
        QCOMPARE(paths.attributes,
                 dir.filePath(QStringLiteral("LIVE/USER/Client/0/Profiles/default/attributes.xml")));
        QCOMPARE(paths.actionMaps,
                 dir.filePath(QStringLiteral("LIVE/USER/Client/0/Profiles/default/actionmaps.xml")));
        QCOMPARE(paths.mappingsDir, dir.filePath(QStringLiteral("LIVE/USER/Client/0/controls/mappings")));
        const ProfilePaths fresh = profilePaths(dir.filePath(QStringLiteral("PTU")));
        QCOMPARE(fresh.dir, dir.filePath(QStringLiteral("PTU/user/client/0/Profiles/default")));
        QVERIFY(profilePaths(QString()).dir.isEmpty());
    }
};

QTEST_GUILESS_MAIN(TestGameSettings)
#include "tst_gamesettings.moc"
