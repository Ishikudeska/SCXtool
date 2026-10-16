#pragma once

#include "core/gamesettings/Bindings.h"

#include <QByteArray>
#include <QString>

#include <memory>
#include <optional>
#include <utility>
#include <vector>

namespace pugi {
class xml_document;
class xml_node;
}

namespace core::gamesettings {

struct Rebind
{
    QString input;
    QString activationMode;
    int multiTap = 0;
};

struct DeviceInfo
{
    QString type;
    int instance = 1;
    QString product;
};

struct BoundAction
{
    QString map;
    QString action;
    Device device = Device::Keyboard;
    int instance = 1;
    Rebind rebind;
};

class ActionMapsFile
{
public:
    ActionMapsFile();
    ~ActionMapsFile();
    ActionMapsFile(ActionMapsFile &&) noexcept;
    ActionMapsFile &operator=(ActionMapsFile &&) noexcept;
    ActionMapsFile(const ActionMapsFile &other);
    ActionMapsFile &operator=(const ActionMapsFile &other);

    static std::optional<ActionMapsFile> parse(const QByteArray &xml, QString *error = nullptr);
    static std::optional<ActionMapsFile> load(const QString &path, QString *error = nullptr);
    static ActionMapsFile empty();

    QByteArray serialize() const;
    bool save(const QString &path, QString *error = nullptr) const;

    std::optional<Rebind> rebind(const QString &map, const QString &action, Device device,
                                 int instance) const;
    void setRebind(const QString &map, const QString &action, Device device, int instance,
                   const Rebind &rebind);
    bool removeRebind(const QString &map, const QString &action, Device device, int instance);
    int removeRebinds(Device device, int instance);
    std::vector<BoundAction> rebinds() const;

    std::optional<QString> option(const QString &type, int instance, const QString &group,
                                  const QString &attribute) const;
    void setOption(const QString &type, int instance, const QString &group, const QString &attribute,
                   const QString &value);
    bool removeOption(const QString &type, int instance, const QString &group, const QString &attribute);
    std::optional<std::vector<std::pair<double, double>>> curve(const QString &type, int instance,
                                                                const QString &group) const;
    void setCurve(const QString &type, int instance, const QString &group,
                  const std::vector<std::pair<double, double>> &points);
    bool removeCurve(const QString &type, int instance, const QString &group);

    std::optional<QString> deviceOption(const QString &product, const QString &input,
                                        const QString &attribute) const;
    void setDeviceOption(const QString &product, const QString &input, const QString &attribute,
                         const QString &value);
    bool removeDeviceOption(const QString &product, const QString &input, const QString &attribute);

    std::vector<DeviceInfo> devices() const;

private:
    pugi::xml_node profile() const;
    pugi::xml_node findOptions(const QString &type, int instance) const;
    pugi::xml_node ensureOptions(const QString &type, int instance);
    pugi::xml_node findRebind(const QString &map, const QString &action, Device device, int instance) const;
    void prune(pugi::xml_node node);

    std::unique_ptr<pugi::xml_document> doc_;
};

}
