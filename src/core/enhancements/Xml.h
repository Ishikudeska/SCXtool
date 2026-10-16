#pragma once

#include <QString>

#include <pugixml.hpp>

#include <memory>
#include <optional>
#include <string_view>
#include <vector>

namespace core::enh {

using Node = pugi::xml_node;

class XmlDoc
{
public:
    XmlDoc() = default;
    static XmlDoc load(const QString &path);
    static XmlDoc parse(std::string_view text);

    explicit operator bool() const { return doc_ != nullptr; }
    Node root() const { return doc_ ? doc_->document_element() : Node(); }

private:
    std::shared_ptr<pugi::xml_document> doc_;
};

inline std::string_view tag(Node n)
{
    return n.name();
}

std::optional<std::string_view> get(Node n, const char *name);
std::string_view getOr(Node n, const char *name, std::string_view fallback = {});

QString qs(std::string_view utf8);
inline QString qs(const std::optional<std::string_view> &v)
{
    return v ? qs(*v) : QString();
}

Node find(Node n, std::string_view path);
std::vector<Node> findAll(Node n, std::string_view path);

std::vector<Node> iter(Node n, std::string_view tag = {});
template <class Visit> void forEachElement(Node n, Visit &&visit)
{
    if (!n)
        return;
    Node cur = n;
    while (true) {
        if (cur.type() == pugi::node_element && !visit(cur))
            return;
        if (Node child = cur.first_child()) {
            cur = child;
            continue;
        }
        while (cur != n && !cur.next_sibling())
            cur = cur.parent();
        if (cur == n)
            return;
        cur = cur.next_sibling();
    }
}

std::vector<Node> children(Node n);

Node findDescendant(Node root, std::string_view tagName);
Node findByType(Node root, std::string_view typeName);
std::optional<std::string_view> attrOf(Node root, std::string_view tagName, const char *attr);

}
