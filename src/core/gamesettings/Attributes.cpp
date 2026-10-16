#include "core/gamesettings/Attributes.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>

#include <pugixml.hpp>

#include <cmath>

namespace core::gamesettings {

namespace {

bool isPcPlatform(const QString &platform)
{
    if (platform.isEmpty())
        return true;
    const QStringList parts = platform.split(u'|');
    for (const QString &part : parts)
        if (part.trimmed().compare(QLatin1StringView("pc"), Qt::CaseInsensitive) == 0)
            return true;
    return false;
}

QString attr(const pugi::xml_node &node, const char *name)
{
    return QString::fromUtf8(node.attribute(name).value());
}

QByteArray escape(const QString &text)
{
    QByteArray out;
    const QByteArray utf8 = text.toUtf8();
    out.reserve(utf8.size());
    for (const char c : utf8) {
        switch (c) {
        case '&':
            out += "&amp;";
            break;
        case '<':
            out += "&lt;";
            break;
        case '>':
            out += "&gt;";
            break;
        case '"':
            out += "&quot;";
            break;
        default:
            out += c;
        }
    }
    return out;
}

}

AttributeDefaults parseAttributeDefaults(const QByteArray &xml, QString *error)
{
    AttributeDefaults out;
    pugi::xml_document doc;
    const pugi::xml_parse_result parsed = doc.load_buffer(xml.constData(), static_cast<size_t>(xml.size()));
    if (!parsed) {
        if (error)
            *error = QString::fromUtf8(parsed.description());
        return out;
    }
    const pugi::xml_node root = doc.child("Attributes");
    out.version = attr(root, "Version");
    QMap<QString, bool> seen;
    for (const pugi::xml_node node : root.children("Attr")) {
        const QString name = attr(node, "name");
        if (name.isEmpty() || seen.contains(name) || !isPcPlatform(attr(node, "platform")))
            continue;
        seen.insert(name, true);
        AttributeDef def;
        def.name = name;
        def.cvar = attr(node, "cvar");
        def.defaultValue = attr(node, "value");
        def.preview = node.attribute("preview").as_int() != 0;
        def.confirmation = node.attribute("confirmation").as_int() != 0;
        out.attributes.push_back(std::move(def));
    }
    return out;
}

QString formatNumber(double value)
{
    if (!std::isfinite(value))
        return QStringLiteral("0");
    if (value == std::floor(value) && std::fabs(value) < 1e15)
        return QString::number(static_cast<qint64>(value));
    QString text = QString::number(value, 'f', 6);
    while (text.endsWith(u'0'))
        text.chop(1);
    if (text.endsWith(u'.'))
        text.chop(1);
    return text == QLatin1StringView("-0") ? QStringLiteral("0") : text;
}

bool sameValue(const QString &a, const QString &b)
{
    if (a == b)
        return true;
    bool okA = false, okB = false;
    const double x = a.toDouble(&okA);
    const double y = b.toDouble(&okB);
    return okA && okB && std::fabs(x - y) <= 1e-6 * std::max(1.0, std::fabs(y));
}

std::optional<AttributesFile> AttributesFile::parse(const QByteArray &xml, QString *error)
{
    pugi::xml_document doc;
    const pugi::xml_parse_result parsed = doc.load_buffer(xml.constData(), static_cast<size_t>(xml.size()));
    if (!parsed) {
        if (error)
            *error = QString::fromUtf8(parsed.description());
        return std::nullopt;
    }
    const pugi::xml_node root = doc.child("Attributes");
    if (!root) {
        if (error)
            *error = QStringLiteral("no <Attributes> element");
        return std::nullopt;
    }
    AttributesFile file;
    file.version_ = attr(root, "Version");
    for (const pugi::xml_node node : root.children("Attr")) {
        const QString name = attr(node, "name");
        if (!name.isEmpty())
            file.values_.insert(name, attr(node, "value"));
    }
    return file;
}

std::optional<AttributesFile> AttributesFile::load(const QString &path, QString *error)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        if (error)
            *error = file.errorString();
        return std::nullopt;
    }
    return parse(file.readAll(), error);
}

QByteArray AttributesFile::serialize() const
{
    QByteArray out = "<Attributes Version=\"" + escape(version_) + "\">\r\n";
    for (auto it = values_.cbegin(); it != values_.cend(); ++it)
        out += " <Attr name=\"" + escape(it.key()) + "\" value=\"" + escape(it.value()) + "\"/>\r\n";
    out += "</Attributes>\r\n";
    return out;
}

bool AttributesFile::save(const QString &path, QString *error) const
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

std::optional<QString> AttributesFile::value(const QString &name) const
{
    const auto it = values_.constFind(name);
    if (it == values_.cend())
        return std::nullopt;
    return *it;
}

void AttributesFile::set(const QString &name, const QString &value)
{
    values_.insert(name, value);
}

bool AttributesFile::remove(const QString &name)
{
    return values_.remove(name) > 0;
}

}
