#pragma once

#include <atomic>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace engine::p4k {

class TreeIndex
{
public:
    using NodeId = std::uint32_t;
    static constexpr NodeId kNone = 0xFFFFFFFFu;

    struct Item
    {
        std::string_view path;
        std::uint64_t size = 0;
        std::uint64_t packed = 0;
    };

    struct Node
    {
        std::string_view name;
        NodeId parent = kNone;
        std::uint32_t row = 0;
        std::uint32_t firstChild = 0;
        std::uint32_t childCount = 0;
        std::uint32_t item = kNone;
        std::uint32_t files = 0;
        std::uint64_t size = 0;
        std::uint64_t packed = 0;
    };

    static std::optional<TreeIndex> build(std::span<const Item> items,
                                          const std::atomic<bool> *cancel = nullptr);

    NodeId root() const { return 0; }
    std::size_t nodeCount() const { return nodes_.size(); }
    const Node &node(NodeId id) const { return nodes_[id]; }
    bool isFolder(NodeId id) const { return nodes_[id].item == kNone; }
    NodeId child(NodeId folder, std::uint32_t row) const
    {
        return children_[nodes_[folder].firstChild + row];
    }

    std::string path(NodeId id) const;
    std::optional<NodeId> find(std::string_view path) const;
    void collectItems(NodeId id, std::vector<std::uint32_t> &out) const;
    NodeId nodeOfItem(std::uint32_t index) const { return itemNodes_[index]; }

private:
    std::vector<Node> nodes_;
    std::vector<NodeId> children_;
    std::vector<NodeId> itemNodes_;
};

}
