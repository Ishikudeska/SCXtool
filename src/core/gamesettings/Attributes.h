#pragma once

#include <QByteArray>
#include <QMap>
#include <QString>

#include <optional>
#include <vector>

namespace core::gamesettings {

struct AttributeDef
{
    QString name;
    QString cvar;
    QString defaultValue;
    bool preview = false;
    bool confirmation = false;
};

struct AttributeDefaults
{
    QString version;
    std::vector<AttributeDef> attributes;
};

AttributeDefaults parseAttributeDefaults(const QByteArray &xml, QString *error = nullptr);

QString formatNumber(double value);

bool sameValue(const QString &a, const QString &b);

class AttributesFile
{
public:
    static std::optional<AttributesFile> parse(const QByteArray &xml, QString *error = nullptr);
    static std::optional<AttributesFile> load(const QString &path, QString *error = nullptr);

    QByteArray serialize() const;
    bool save(const QString &path, QString *error = nullptr) const;

    QString version() const { return version_; }
    void setVersion(const QString &version) { version_ = version; }

    std::optional<QString> value(const QString &name) const;
    void set(const QString &name, const QString &value);
    bool remove(const QString &name);
    const QMap<QString, QString> &values() const { return values_; }

private:
    QString version_;
    QMap<QString, QString> values_;
};

}
