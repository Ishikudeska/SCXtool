#pragma once

#include "core/loadout/Catalog.h"

#include <QList>
#include <QPair>
#include <QString>

#include <vector>

namespace core::loadout {

struct Slot
{
    Port port;
    QString path;
    const Item *item = nullptr;
    std::vector<Slot> children;
};

const Item *stockItem(const Catalog &catalog, const QStringList &shipTags, const Port *port,
                      const LoadoutEntry &entry);

class Loadout
{
public:
    Loadout() = default;
    Loadout(const Catalog &catalog, const Ship &ship);

    const Ship *ship() const { return ship_; }
    const Catalog *catalog() const { return catalog_; }
    const std::vector<Slot> &roots() const { return slots_; }
    const Slot *find(const QString &path) const;

    bool setItem(const QString &path, const QString &itemId);
    std::vector<const Item *> compatible(const QString &path) const;

    QList<QPair<QString, QString>> entries() const;
    int apply(const QList<QPair<QString, QString>> &entries);
    QList<QPair<QString, QString>> changes() const;

    template <class Visit> void forEach(Visit &&visit) const { forEachIn(slots_, 0, visit); }

private:
    template <class Visit> static void forEachIn(const std::vector<Slot> &list, int depth, Visit &visit)
    {
        for (const Slot &s : list) {
            visit(s, depth);
            forEachIn(s.children, depth + 1, visit);
        }
    }
    Slot *findMutable(const QString &path);
    void fill(Slot &slot, const Item *item, const std::vector<LoadoutEntry> *overrides);

    const Catalog *catalog_ = nullptr;
    const Ship *ship_ = nullptr;
    std::vector<Slot> slots_;
};

}
