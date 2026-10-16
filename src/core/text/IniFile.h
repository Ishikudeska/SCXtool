#pragma once

#include <QHash>
#include <QList>
#include <QString>

#include <utility>

namespace core {

class IniMap
{
public:
    using Entry = std::pair<QString, QString>;

    void insert(const QString &key, const QString &value);
    bool remove(const QString &key);
    void clear();
    void reserve(qsizetype n);

    bool contains(const QString &key) const { return index_.contains(key); }
    const QString *find(const QString &key) const;
    QString value(const QString &key, const QString &fallback = {}) const;

    qsizetype size() const { return entries_.size(); }
    bool isEmpty() const { return entries_.isEmpty(); }

    const QList<Entry> &entries() const { return entries_; }
    auto begin() const { return entries_.cbegin(); }
    auto end() const { return entries_.cend(); }

private:
    QList<Entry> entries_;
    QHash<QString, qsizetype> index_;
};

IniMap parseIni(const QString &text, bool stripValues = true);

IniMap loadIni(const QString &path, bool stripValues = true);

QString formatIni(const IniMap &map);

bool writeIniFile(const QString &path, const IniMap &map);

QByteArray toCrlfUtf8(const QString &text);

}
