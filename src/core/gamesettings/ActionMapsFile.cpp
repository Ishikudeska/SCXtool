#include "core/gamesettings/ActionMapsFile.h"

#include "core/gamesettings/Attributes.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>

#include <pugixml.hpp>

#include <sstream>

namespace core::gamesettings {

namespace {

QString attr(const pugi::xml_node &node, const char *name)
{
    return QString::fromUtf8(node.attribute(name).value());
}

QByteArray utf8(const QString &text)
{
    return text.toUtf8();
}

void setAttr(pugi::xml_node node, const char *name, const QString &value)
{
    pugi::xml_attribute a = node.attribute(name);
    if (!a)
        a = node.append_attribute(name);
    a.set_value(utf8(value).constData());
}

pugi::xml_node childWithAttr(const pugi::xml_node &parent, const char *element, const char *name,
                             const QString &value)
{
    const QByteArray v = utf8(value);
    for (pugi::xml_node child : parent.children(element))
        if (v == child.attribute(name).value())
            return child;
    return {};
}

bool matchesDevice(const pugi::xml_node &rebind, Device device, int instance)
{
    const std::optional<ParsedInput> parsed = parseInput(attr(rebind, "input"));
    return parsed && parsed->device == device && parsed->instance == instance;
}

const char *kEmptyProfile = "<ActionMaps>\n"
                            " <ActionProfiles version=\"1\" optionsVersion=\"2\" rebindVersion=\"2\" "
                            "profileName=\"default\">\n"
                            "  <modifiers />\n"
                            " </ActionProfiles>\n"
                            "</ActionMaps>\n";

}

ActionMapsFile::ActionMapsFile() : doc_(std::make_unique<pugi::xml_document>())
{}
ActionMapsFile::~ActionMapsFile() = default;
ActionMapsFile::ActionMapsFile(ActionMapsFile &&) noexcept = default;
ActionMapsFile &ActionMapsFile::operator=(ActionMapsFile &&) noexcept = default;

ActionMapsFile::ActionMapsFile(const ActionMapsFile &other) : doc_(std::make_unique<pugi::xml_document>())
{
    doc_->reset(*other.doc_);
}

ActionMapsFile &ActionMapsFile::operator=(const ActionMapsFile &other)
{
    if (this != &other)
        doc_->reset(*other.doc_);
    return *this;
}

std::optional<ActionMapsFile> ActionMapsFile::parse(const QByteArray &xml, QString *error)
{
    ActionMapsFile file;
    const pugi::xml_parse_result parsed =
        file.doc_->load_buffer(xml.constData(), static_cast<size_t>(xml.size()), pugi::parse_default);
    if (!parsed) {
        if (error)
            *error = QString::fromUtf8(parsed.description());
        return std::nullopt;
    }
    if (!file.doc_->child("ActionMaps")) {
        if (error)
            *error = QStringLiteral("no <ActionMaps> element");
        return std::nullopt;
    }
    return file;
}

std::optional<ActionMapsFile> ActionMapsFile::load(const QString &path, QString *error)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        if (error)
            *error = file.errorString();
        return std::nullopt;
    }
    return parse(file.readAll(), error);
}

ActionMapsFile ActionMapsFile::empty()
{
    ActionMapsFile file;
    file.doc_->load_string(kEmptyProfile);
    return file;
}

QByteArray ActionMapsFile::serialize() const
{
    std::ostringstream stream;
    doc_->save(stream, " ", pugi::format_indent | pugi::format_no_declaration, pugi::encoding_utf8);
    QByteArray out = QByteArray::fromStdString(stream.str());
    out.replace("\r\n", "\n");
    out.replace("\n", "\r\n");
    return out;
}

bool ActionMapsFile::save(const QString &path, QString *error) const
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        if (error)
            *error = file.errorString();
        return false;
    }
    file.write(serialize());
    if (!file.commit()) {
        if (error)
            *error = file.errorString();
        return false;
    }
    return true;
}

pugi::xml_node ActionMapsFile::profile() const
{
    pugi::xml_node root = doc_->child("ActionMaps");
    if (!root)
        root = doc_->append_child("ActionMaps");
    pugi::xml_node first;
    for (pugi::xml_node p : root.children("ActionProfiles")) {
        if (!first)
            first = p;
        if (QLatin1StringView(p.attribute("profileName").value()) == QLatin1StringView("default"))
            return p;
    }
    return first ? first : root;
}

pugi::xml_node ActionMapsFile::findRebind(const QString &map, const QString &action, Device device,
                                          int instance) const
{
    const pugi::xml_node mapNode = childWithAttr(profile(), "actionmap", "name", map);
    const pugi::xml_node actionNode = childWithAttr(mapNode, "action", "name", action);
    for (pugi::xml_node rebind : actionNode.children("rebind"))
        if (matchesDevice(rebind, device, instance))
            return rebind;
    return {};
}

std::optional<Rebind> ActionMapsFile::rebind(const QString &map, const QString &action, Device device,
                                             int instance) const
{
    const pugi::xml_node node = findRebind(map, action, device, instance);
    if (!node)
        return std::nullopt;
    Rebind out;
    out.input = parseInput(attr(node, "input"))->input;
    out.activationMode = attr(node, "activationMode");
    out.multiTap = node.attribute("multiTap").as_int();
    return out;
}

void ActionMapsFile::setRebind(const QString &map, const QString &action, Device device, int instance,
                               const Rebind &rebind)
{
    pugi::xml_node p = profile();
    pugi::xml_node mapNode = childWithAttr(p, "actionmap", "name", map);
    if (!mapNode) {
        mapNode = p.append_child("actionmap");
        setAttr(mapNode, "name", map);
    }
    pugi::xml_node actionNode = childWithAttr(mapNode, "action", "name", action);
    if (!actionNode) {
        actionNode = mapNode.append_child("action");
        setAttr(actionNode, "name", action);
    }
    pugi::xml_node node = findRebind(map, action, device, instance);
    if (!node)
        node = actionNode.append_child("rebind");
    setAttr(node, "input", inputPrefix(device, instance) + blankInput(rebind.input));
    node.remove_attribute("device");
    if (rebind.activationMode.isEmpty())
        node.remove_attribute("activationMode");
    else
        setAttr(node, "activationMode", rebind.activationMode);
    if (rebind.multiTap > 1)
        setAttr(node, "multiTap", QString::number(rebind.multiTap));
    else
        node.remove_attribute("multiTap");
}

void ActionMapsFile::prune(pugi::xml_node node)
{
    while (node && !node.first_child() &&
           QLatin1StringView(node.name()) != QLatin1StringView("ActionProfiles") &&
           QLatin1StringView(node.name()) != QLatin1StringView("ActionMaps") &&
           QLatin1StringView(node.name()) != QLatin1StringView("modifiers") &&
           QLatin1StringView(node.name()) != QLatin1StringView("options")) {
        pugi::xml_node parent = node.parent();
        parent.remove_child(node);
        node = parent;
    }
}

bool ActionMapsFile::removeRebind(const QString &map, const QString &action, Device device, int instance)
{
    pugi::xml_node node = findRebind(map, action, device, instance);
    if (!node)
        return false;
    pugi::xml_node parent = node.parent();
    parent.remove_child(node);
    prune(parent);
    return true;
}

int ActionMapsFile::removeRebinds(Device device, int instance)
{
    int removed = 0;
    for (const BoundAction &bound : rebinds())
        if (bound.device == device && bound.instance == instance &&
            removeRebind(bound.map, bound.action, device, instance))
            ++removed;
    return removed;
}

std::vector<BoundAction> ActionMapsFile::rebinds() const
{
    std::vector<BoundAction> out;
    for (const pugi::xml_node mapNode : profile().children("actionmap"))
        for (const pugi::xml_node actionNode : mapNode.children("action"))
            for (const pugi::xml_node node : actionNode.children("rebind")) {
                const std::optional<ParsedInput> parsed = parseInput(attr(node, "input"));
                if (!parsed)
                    continue;
                out.push_back(
                    {attr(mapNode, "name"),
                     attr(actionNode, "name"),
                     parsed->device,
                     parsed->instance,
                     {parsed->input, attr(node, "activationMode"), node.attribute("multiTap").as_int()}});
            }
    return out;
}

pugi::xml_node ActionMapsFile::findOptions(const QString &type, int instance) const
{
    const QByteArray t = utf8(type);
    for (pugi::xml_node node : profile().children("options"))
        if (t == node.attribute("type").value() && node.attribute("instance").as_int(1) == instance)
            return node;
    return {};
}

pugi::xml_node ActionMapsFile::ensureOptions(const QString &type, int instance)
{
    pugi::xml_node node = findOptions(type, instance);
    if (node)
        return node;
    pugi::xml_node p = profile();
    pugi::xml_node before = p.child("modifiers");
    if (!before)
        before = p.child("actionmap");
    node = before ? p.insert_child_before("options", before) : p.append_child("options");
    setAttr(node, "type", type);
    setAttr(node, "instance", QString::number(instance));
    return node;
}

std::optional<QString> ActionMapsFile::option(const QString &type, int instance, const QString &group,
                                              const QString &attribute) const
{
    const pugi::xml_node groupNode = findOptions(type, instance).child(utf8(group).constData());
    const pugi::xml_attribute a = groupNode.attribute(utf8(attribute).constData());
    if (!a)
        return std::nullopt;
    return QString::fromUtf8(a.value());
}

void ActionMapsFile::setOption(const QString &type, int instance, const QString &group,
                               const QString &attribute, const QString &value)
{
    pugi::xml_node options = ensureOptions(type, instance);
    const QByteArray g = utf8(group);
    pugi::xml_node groupNode = options.child(g.constData());
    if (!groupNode)
        groupNode = options.append_child(g.constData());
    setAttr(groupNode, utf8(attribute).constData(), value);
}

bool ActionMapsFile::removeOption(const QString &type, int instance, const QString &group,
                                  const QString &attribute)
{
    pugi::xml_node groupNode = findOptions(type, instance).child(utf8(group).constData());
    if (!groupNode || !groupNode.remove_attribute(utf8(attribute).constData()))
        return false;
    if (!groupNode.first_attribute() && !groupNode.first_child())
        groupNode.parent().remove_child(groupNode);
    return true;
}

std::optional<std::vector<std::pair<double, double>>> ActionMapsFile::curve(const QString &type, int instance,
                                                                            const QString &group) const
{
    const pugi::xml_node curveNode =
        findOptions(type, instance).child(utf8(group).constData()).child("nonlinearity_curve");
    if (!curveNode)
        return std::nullopt;
    std::vector<std::pair<double, double>> points;
    for (const pugi::xml_node point : curveNode.children("point"))
        points.emplace_back(point.attribute("in").as_double(), point.attribute("out").as_double());
    return points;
}

void ActionMapsFile::setCurve(const QString &type, int instance, const QString &group,
                              const std::vector<std::pair<double, double>> &points)
{
    pugi::xml_node options = ensureOptions(type, instance);
    const QByteArray g = utf8(group);
    pugi::xml_node groupNode = options.child(g.constData());
    if (!groupNode)
        groupNode = options.append_child(g.constData());
    groupNode.remove_child("nonlinearity_curve");
    pugi::xml_node curveNode = groupNode.append_child("nonlinearity_curve");
    for (const auto &[in, out] : points) {
        pugi::xml_node point = curveNode.append_child("point");
        setAttr(point, "in", formatNumber(in));
        setAttr(point, "out", formatNumber(out));
    }
}

bool ActionMapsFile::removeCurve(const QString &type, int instance, const QString &group)
{
    pugi::xml_node groupNode = findOptions(type, instance).child(utf8(group).constData());
    if (!groupNode || !groupNode.remove_child("nonlinearity_curve"))
        return false;
    if (!groupNode.first_attribute() && !groupNode.first_child())
        groupNode.parent().remove_child(groupNode);
    return true;
}

std::optional<QString> ActionMapsFile::deviceOption(const QString &product, const QString &input,
                                                    const QString &attribute) const
{
    const pugi::xml_node device = childWithAttr(profile(), "deviceoptions", "name", product);
    const pugi::xml_attribute a =
        childWithAttr(device, "option", "input", input).attribute(utf8(attribute).constData());
    if (!a)
        return std::nullopt;
    return QString::fromUtf8(a.value());
}

void ActionMapsFile::setDeviceOption(const QString &product, const QString &input, const QString &attribute,
                                     const QString &value)
{
    pugi::xml_node p = profile();
    pugi::xml_node device = childWithAttr(p, "deviceoptions", "name", product);
    if (!device) {
        pugi::xml_node before = p.child("options");
        if (!before)
            before = p.child("modifiers");
        device = before ? p.insert_child_before("deviceoptions", before) : p.append_child("deviceoptions");
        setAttr(device, "name", product);
    }
    pugi::xml_node option = childWithAttr(device, "option", "input", input);
    if (!option) {
        option = device.append_child("option");
        setAttr(option, "input", input);
    }
    setAttr(option, utf8(attribute).constData(), value);
}

bool ActionMapsFile::removeDeviceOption(const QString &product, const QString &input,
                                        const QString &attribute)
{
    pugi::xml_node device = childWithAttr(profile(), "deviceoptions", "name", product);
    pugi::xml_node option = childWithAttr(device, "option", "input", input);
    if (!option || !option.remove_attribute(utf8(attribute).constData()))
        return false;
    if (std::distance(option.attributes_begin(), option.attributes_end()) <= 1)
        device.remove_child(option);
    if (!device.first_child())
        device.parent().remove_child(device);
    return true;
}

std::vector<DeviceInfo> ActionMapsFile::devices() const
{
    std::vector<DeviceInfo> out;
    for (const pugi::xml_node node : profile().children("options"))
        out.push_back({attr(node, "type"), node.attribute("instance").as_int(1), attr(node, "Product")});
    return out;
}

}
