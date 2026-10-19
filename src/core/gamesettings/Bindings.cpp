#include "core/gamesettings/Bindings.h"

#include <QRegularExpression>

#include <pugixml.hpp>

namespace core::gamesettings {

namespace {

QString attr(const pugi::xml_node &node, const char *name)
{
    return QString::fromUtf8(node.attribute(name).value());
}

QString gameText(const QString &ref, const TextLookup &text)
{
    if (ref.isEmpty())
        return {};
    return resolveText(ref.startsWith(u'@') ? ref : u'@' + ref, text);
}

int showFlag(const pugi::xml_node &node, const char *name)
{
    return node.attribute(name) ? node.attribute(name).as_int() : 0;
}

void readCurve(const pugi::xml_node &node, std::vector<std::pair<double, double>> &curve)
{
    for (const pugi::xml_node point : node.child("nonlinearity_curve").children("point"))
        curve.emplace_back(point.attribute("in").as_double(), point.attribute("out").as_double());
}

void readOptionGroup(const pugi::xml_node &node, int depth, int parent, OptionTree &tree,
                     const TextLookup &text)
{
    for (const pugi::xml_node child : node.children("optiongroup")) {
        OptionNode option;
        option.name = attr(child, "name");
        option.label = gameText(attr(child, "UILabel"), text);
        if (option.label.isEmpty())
            option.label = labelFromName(option.name);
        option.invertCvar = attr(child, "invert_cvar");
        option.depth = depth;
        option.parent = parent;
        option.showInvert = showFlag(child, "UIShowInvert");
        option.showCurve = showFlag(child, "UIShowCurve");
        option.showSensitivity = showFlag(child, "UIShowSensitivity");
        if (child.attribute("invert"))
            option.invert = child.attribute("invert").as_int() != 0;
        if (child.attribute("exponent"))
            option.exponent = child.attribute("exponent").as_double();
        readCurve(child, option.curve);
        const int index = static_cast<int>(tree.nodes.size());
        tree.nodes.push_back(std::move(option));
        readOptionGroup(child, depth + 1, index, tree, text);
    }
}

void addDefault(ActionDef &action, const QString &attribute, const QString &input,
                const QString &activationMode, int multiTap)
{
    if (input.trimmed().isEmpty())
        return;
    const std::optional<Device> device = deviceFromId(attribute);
    if (!device)
        return;
    action.defaults.push_back({defaultDevice(attribute, input), input.trimmed(), activationMode, multiTap});
}

QString prettyName(const QString &input)
{
    if (input.size() == 1)
        return input.toUpper();
    QString out = input;
    out.replace(u'_', u' ');
    if (!out.isEmpty())
        out[0] = out[0].toUpper();
    return out;
}

}

QString deviceId(Device device)
{
    switch (device) {
    case Device::Keyboard:
        return QStringLiteral("keyboard");
    case Device::Mouse:
        return QStringLiteral("mouse");
    case Device::Gamepad:
        return QStringLiteral("gamepad");
    case Device::Joystick:
        return QStringLiteral("joystick");
    }
    return {};
}

std::optional<Device> deviceFromId(const QString &id)
{
    if (id == u"keyboard")
        return Device::Keyboard;
    if (id == u"mouse")
        return Device::Mouse;
    if (id == u"gamepad")
        return Device::Gamepad;
    if (id == u"joystick")
        return Device::Joystick;
    return std::nullopt;
}

QString inputPrefix(Device device, int instance)
{
    const QString n = QString::number(instance);
    switch (device) {
    case Device::Keyboard:
        return QStringLiteral("kb") + n + u'_';
    case Device::Mouse:
        return QStringLiteral("mo") + n + u'_';
    case Device::Gamepad:
        return QStringLiteral("gp") + n + u'_';
    case Device::Joystick:
        return QStringLiteral("js") + n + u'_';
    }
    return {};
}

bool isMouseInput(const QString &input)
{
    const QString last = input.section(u'+', -1).trimmed();
    return last.startsWith(QLatin1StringView("mouse")) || last.startsWith(QLatin1StringView("maxis")) ||
           last.startsWith(QLatin1StringView("mwheel"));
}

Device defaultDevice(const QString &attribute, const QString &input)
{
    const Device device = deviceFromId(attribute).value_or(Device::Keyboard);
    if (device == Device::Keyboard && isMouseInput(input))
        return Device::Mouse;
    return device;
}

std::optional<ParsedInput> parseInput(const QString &prefixed)
{
    static const QRegularExpression pattern(QStringLiteral("^(kb|mo|gp|js)(\\d+)_(.*)$"));
    const QRegularExpressionMatch m = pattern.match(prefixed);
    if (!m.hasMatch())
        return std::nullopt;
    ParsedInput out;
    const QString kind = m.captured(1);
    out.device = kind == u"kb"   ? Device::Keyboard
                 : kind == u"mo" ? Device::Mouse
                 : kind == u"gp" ? Device::Gamepad
                                 : Device::Joystick;
    out.instance = m.captured(2).toInt();
    out.input = m.captured(3).trimmed();
    return out;
}

QString blankInput(const QString &input)
{
    return input.trimmed().isEmpty() ? QStringLiteral(" ") : input.trimmed();
}

int OptionTree::find(const QString &name) const
{
    for (std::size_t i = 0; i < nodes.size(); ++i)
        if (nodes[i].name == name)
            return static_cast<int>(i);
    return -1;
}

std::optional<bool> OptionTree::inheritedInvert(int node) const
{
    for (int i = node; i >= 0; i = nodes[std::size_t(i)].parent)
        if (nodes[std::size_t(i)].invert)
            return nodes[std::size_t(i)].invert;
    return std::nullopt;
}

std::optional<double> OptionTree::inheritedExponent(int node) const
{
    for (int i = node; i >= 0; i = nodes[std::size_t(i)].parent)
        if (nodes[std::size_t(i)].exponent)
            return nodes[std::size_t(i)].exponent;
    return std::nullopt;
}

const OptionTree *BindingsCatalog::tree(const QString &type) const
{
    for (const OptionTree &t : optionTrees)
        if (t.type == type)
            return &t;
    return nullptr;
}

BindingsCatalog parseDefaultProfile(const QByteArray &xml, const TextLookup &text, QString *error)
{
    BindingsCatalog catalog;
    pugi::xml_document doc;
    const pugi::xml_parse_result parsed = doc.load_buffer(xml.constData(), static_cast<size_t>(xml.size()));
    if (!parsed) {
        if (error)
            *error = QString::fromUtf8(parsed.description());
        return catalog;
    }
    const pugi::xml_node profile = doc.child("profile");

    for (const pugi::xml_node mode : profile.child("ActivationModes").children("ActivationMode"))
        catalog.activationModes << attr(mode, "name");

    for (const pugi::xml_node treeNode : profile.children("optiontree")) {
        OptionTree tree;
        tree.type = attr(treeNode, "type");
        tree.instances = std::max(1, treeNode.attribute("instances").as_int(1));
        tree.sensitivityMin = treeNode.attribute("UISensitivityMin").as_double();
        tree.sensitivityMax = treeNode.attribute("UISensitivityMax").as_double();
        readOptionGroup(treeNode, 0, -1, tree, text);
        catalog.optionTrees.push_back(std::move(tree));
    }

    for (const pugi::xml_node mapNode : profile.children("actionmap")) {
        ActionMapDef map;
        map.name = attr(mapNode, "name");
        map.label = gameText(attr(mapNode, "UILabel"), text);
        map.category = gameText(attr(mapNode, "UICategory"), text);
        if (attr(mapNode, "UILabel").isEmpty())
            continue;
        for (const pugi::xml_node actionNode : mapNode.children("action")) {
            const QString labelRef = attr(actionNode, "UILabel");
            if (labelRef.isEmpty())
                continue;
            ActionDef action;
            action.map = map.name;
            action.name = attr(actionNode, "name");
            action.label = gameText(labelRef, text);
            if (action.label.isEmpty())
                action.label = labelFromName(action.name);
            action.description = gameText(attr(actionNode, "UIDescription"), text);
            action.activationMode = attr(actionNode, "activationMode");
            action.optionGroup = attr(actionNode, "optionGroup");
            for (const char *device : {"keyboard", "mouse", "gamepad", "joystick"})
                addDefault(action, QString::fromLatin1(device), attr(actionNode, device),
                           action.activationMode, actionNode.attribute("multiTap").as_int());
            for (const pugi::xml_node child : actionNode.children()) {
                const QString device = QString::fromUtf8(child.name());
                if (!deviceFromId(device))
                    continue;
                const QString mode =
                    child.attribute("activationMode") ? attr(child, "activationMode") : action.activationMode;
                addDefault(action, device, attr(child, "input"), mode, child.attribute("multiTap").as_int());
                for (const pugi::xml_node data : child.children("inputdata"))
                    addDefault(action, device, attr(data, "input"), mode,
                               data.attribute("multiTap").as_int());
            }
            map.actions.push_back(std::move(action));
        }
        if (!map.actions.empty())
            catalog.maps.push_back(std::move(map));
    }
    return catalog;
}

QString KeyLabels::table(Device device)
{
    switch (device) {
    case Device::Keyboard:
        return QStringLiteral("keyboard");
    case Device::Mouse:
        return QStringLiteral("mouse");
    case Device::Gamepad:
        return QStringLiteral("control_pad");
    case Device::Joystick:
        return QStringLiteral("joystick1");
    }
    return {};
}

KeyLabels KeyLabels::parse(const QByteArray &xml, const TextLookup &text)
{
    KeyLabels out;
    pugi::xml_document doc;
    if (!doc.load_buffer(xml.constData(), static_cast<size_t>(xml.size())))
        return out;
    for (const pugi::xml_node device : doc.child("LocalizedKeys").children("device")) {
        const QString table = attr(device, "name");
        QStringList &names = out.inputs_[table];
        for (const pugi::xml_node key : device.children("Key")) {
            const QString name = attr(key, "name");
            if (name.isEmpty() || names.contains(name))
                continue;
            names << name;
            const QString label = gameText(attr(key, "localizationString"), text);
            if (!label.isEmpty())
                out.labels_.insert(table + u'|' + name, label);
        }
    }
    return out;
}

QString KeyLabels::label(Device device, const QString &input) const
{
    for (const QString &t : {table(device), table(Device::Keyboard)}) {
        const auto it = labels_.constFind(t + u'|' + input);
        if (it != labels_.cend())
            return *it;
    }
    return prettyName(input);
}

QString KeyLabels::describe(Device device, const QString &input) const
{
    if (input.trimmed().isEmpty())
        return {};
    const QStringList parts = input.trimmed().split(u'+', Qt::SkipEmptyParts);
    QStringList labels;
    for (const QString &part : parts)
        labels << label(device, part.trimmed());
    return labels.join(QStringLiteral(" + "));
}

QStringList KeyLabels::inputs(Device device) const
{
    QStringList names = inputs_.value(table(device));
    if (device == Device::Keyboard)
        names.removeIf([](const QString &n) { return isMouseInput(n); });
    return names;
}

QString keyNameFromQt(int qtKey, quint32 nativeScanCode, bool keypad)
{
    struct Pair
    {
        quint32 left;
        quint32 right;
    };
#ifdef Q_OS_WIN
    constexpr Pair shift{0x2A, 0x36}, ctrl{0x1D, 0x11D}, alt{0x38, 0x138};
#else
    constexpr Pair shift{50, 62}, ctrl{37, 105}, alt{64, 108};
#endif
    auto side = [nativeScanCode](const Pair &codes, const char *l, const char *r) {
        return QString::fromLatin1(nativeScanCode == codes.right ? r : l);
    };
    switch (qtKey) {
    case Qt::Key_Shift:
        return side(shift, "lshift", "rshift");
    case Qt::Key_Control:
        return side(ctrl, "lctrl", "rctrl");
    case Qt::Key_Alt:
        return side(alt, "lalt", "ralt");
    case Qt::Key_AltGr:
        return QStringLiteral("ralt");
    default:
        break;
    }
    if (keypad) {
        if (qtKey >= Qt::Key_0 && qtKey <= Qt::Key_9)
            return QStringLiteral("np_") + QChar(u'0' + (qtKey - Qt::Key_0));
        switch (qtKey) {
        case Qt::Key_Slash:
            return QStringLiteral("np_divide");
        case Qt::Key_Asterisk:
            return QStringLiteral("np_multiply");
        case Qt::Key_Minus:
            return QStringLiteral("np_subtract");
        case Qt::Key_Plus:
            return QStringLiteral("np_add");
        case Qt::Key_Period:
        case Qt::Key_Comma:
        case Qt::Key_Delete:
            return QStringLiteral("np_period");
        case Qt::Key_Enter:
        case Qt::Key_Return:
            return QStringLiteral("np_enter");
        default:
            break;
        }
    }
    if (qtKey >= Qt::Key_A && qtKey <= Qt::Key_Z)
        return QString(QChar(u'a' + (qtKey - Qt::Key_A)));
    if (qtKey >= Qt::Key_0 && qtKey <= Qt::Key_9)
        return QString(QChar(u'0' + (qtKey - Qt::Key_0)));
    if (qtKey >= Qt::Key_F1 && qtKey <= Qt::Key_F24)
        return QStringLiteral("f") + QString::number(qtKey - Qt::Key_F1 + 1);
    switch (qtKey) {
    case Qt::Key_Escape:
        return QStringLiteral("escape");
    case Qt::Key_Tab:
    case Qt::Key_Backtab:
        return QStringLiteral("tab");
    case Qt::Key_Backspace:
        return QStringLiteral("backspace");
    case Qt::Key_Return:
        return QStringLiteral("enter");
    case Qt::Key_Enter:
        return QStringLiteral("np_enter");
    case Qt::Key_Insert:
        return QStringLiteral("insert");
    case Qt::Key_Delete:
        return QStringLiteral("delete");
    case Qt::Key_Home:
        return QStringLiteral("home");
    case Qt::Key_End:
        return QStringLiteral("end");
    case Qt::Key_PageUp:
        return QStringLiteral("pgup");
    case Qt::Key_PageDown:
        return QStringLiteral("pgdn");
    case Qt::Key_Left:
        return QStringLiteral("left");
    case Qt::Key_Right:
        return QStringLiteral("right");
    case Qt::Key_Up:
        return QStringLiteral("up");
    case Qt::Key_Down:
        return QStringLiteral("down");
    case Qt::Key_Space:
        return QStringLiteral("space");
    case Qt::Key_Minus:
        return QStringLiteral("minus");
    case Qt::Key_Underscore:
        return QStringLiteral("underline");
    case Qt::Key_Equal:
        return QStringLiteral("equals");
    case Qt::Key_BracketLeft:
        return QStringLiteral("lbracket");
    case Qt::Key_BracketRight:
        return QStringLiteral("rbracket");
    case Qt::Key_Semicolon:
        return QStringLiteral("semicolon");
    case Qt::Key_Colon:
        return QStringLiteral("colon");
    case Qt::Key_Apostrophe:
        return QStringLiteral("apostrophe");
    case Qt::Key_Comma:
        return QStringLiteral("comma");
    case Qt::Key_Period:
        return QStringLiteral("period");
    case Qt::Key_Slash:
        return QStringLiteral("slash");
    case Qt::Key_Backslash:
        return QStringLiteral("backslash");
    case Qt::Key_QuoteLeft:
        return QStringLiteral("grave");
    case Qt::Key_Less:
    case Qt::Key_Greater:
    case Qt::Key_Bar:
        return QStringLiteral("oem_102");
    case Qt::Key_CapsLock:
        return QStringLiteral("capslock");
    case Qt::Key_NumLock:
        return QStringLiteral("numlock");
    case Qt::Key_ScrollLock:
        return QStringLiteral("scrolllock");
    case Qt::Key_Pause:
        return QStringLiteral("pause");
    case Qt::Key_Print:
        return QStringLiteral("print");
    default:
        break;
    }
    return {};
}

QString mouseButtonName(int qtButton)
{
    switch (qtButton) {
    case Qt::LeftButton:
        return QStringLiteral("mouse1");
    case Qt::RightButton:
        return QStringLiteral("mouse2");
    case Qt::MiddleButton:
        return QStringLiteral("mouse3");
    case Qt::BackButton:
        return QStringLiteral("mouse4");
    case Qt::ForwardButton:
        return QStringLiteral("mouse5");
    case Qt::ExtraButton3:
        return QStringLiteral("mouse6");
    case Qt::ExtraButton4:
        return QStringLiteral("mouse7");
    case Qt::ExtraButton5:
        return QStringLiteral("mouse8");
    default:
        break;
    }
    return {};
}

bool isModifierName(const QString &name)
{
    static const QStringList modifiers = {QStringLiteral("lshift"), QStringLiteral("rshift"),
                                          QStringLiteral("lctrl"),  QStringLiteral("rctrl"),
                                          QStringLiteral("lalt"),   QStringLiteral("ralt")};
    return modifiers.contains(name);
}

}
