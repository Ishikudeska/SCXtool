#pragma once

#include "core/enhancements/RecordStore.h"
#include "core/enhancements/Xml.h"
#include "core/text/IniFile.h"

#include <QByteArray>
#include <QHash>
#include <QSet>
#include <QString>
#include <QStringList>

#include <array>
#include <vector>

namespace core::missions {

class TagTable
{
public:
    static TagTable load(const QString &path);
    static TagTable parse(const QByteArray &tsv);

    bool isEmpty() const { return tags_.isEmpty(); }
    QString name(const QString &guid) const;
    QString parent(const QString &guid) const;
    QStringList findByName(const QString &name) const;
    QStringList lineage(const QString &guid) const;
    QStringList children(const QString &guid) const;

private:
    struct Tag
    {
        QString name, parent;
    };
    QHash<QString, Tag> tags_;
    QHash<QString, QStringList> byName_;
    QHash<QString, QStringList> children_;
};

class LocText
{
public:
    explicit LocText(const IniMap &loc);
    QString operator()(QStringView ref) const;

private:
    const IniMap &loc_;
    QHash<QString, const QString *> lower_;
};

struct Place
{
    QString id;
    QString name;
    QString address;
    QString system;
};

enum class TagType { General, Produces, Consumes };
struct TagTerm
{
    QStringList positive, negative;
};
struct TagCondition
{
    TagType type = TagType::General;
    std::vector<TagTerm> terms;
};
struct LocationSearch
{
    std::vector<TagCondition> conditions;

    bool isEmpty() const { return conditions.empty(); }
    QString key() const;
    QStringList positiveTags() const;
};
LocationSearch parseLocationSearch(enh::Node value);
bool matchesGeneral(const LocationSearch &search, const QSet<QString> &tags);

class PlaceIndex
{
public:
    static PlaceIndex build(const enh::RecordStore &store, const TagTable &tags, const LocText &loc);

    bool isEmpty() const { return places_.empty(); }
    const std::vector<Place> &places() const { return places_; }
    std::vector<int> match(const LocationSearch &search) const;
    QStringList systems() const;

private:
    static constexpr int kTagTypes = 3;
    std::vector<Place> places_;
    std::array<std::vector<QSet<QString>>, kTagTypes> tags_;
    std::array<QHash<QString, std::vector<int>>, kTagTypes> byTag_;
};

}
