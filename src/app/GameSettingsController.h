#pragma once

#include "core/gamesettings/ActionMapsFile.h"
#include "core/gamesettings/Attributes.h"
#include "core/gamesettings/Bindings.h"
#include "core/gamesettings/GameProfile.h"
#include "core/gamesettings/SettingsCatalog.h"

#include <QObject>
#include <QStringList>
#include <QTimer>
#include <QVariantList>
#include <QVariantMap>
#include <QtQml/qqmlregistration.h>

#include <memory>
#include <optional>

class AppController;

class GameSettingsController : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString error READ error NOTIFY statusChanged)
    Q_PROPERTY(QString profileDir READ profileDir NOTIFY statusChanged)
    Q_PROPERTY(QVariantList tabs READ tabs NOTIFY catalogChanged)
    Q_PROPERTY(QString tab READ tab WRITE setTab NOTIFY tabChanged)
    Q_PROPERTY(QString search READ search WRITE setSearch NOTIFY filtersChanged)
    Q_PROPERTY(bool modifiedOnly READ modifiedOnly WRITE setModifiedOnly NOTIFY filtersChanged)
    Q_PROPERTY(QVariantList sections READ sections NOTIFY rowsChanged)
    Q_PROPERTY(int pendingCount READ pendingCount NOTIFY pendingChanged)
    Q_PROPERTY(int revision READ revision NOTIFY revisionChanged)
    Q_PROPERTY(bool gameRunning READ gameRunning NOTIFY gameRunningChanged)
    Q_PROPERTY(QVariantList bindDevices READ bindDevices NOTIFY catalogChanged)
    Q_PROPERTY(QString bindDevice READ bindDevice WRITE setBindDevice NOTIFY filtersChanged)
    Q_PROPERTY(QStringList bindCategories READ bindCategories NOTIFY catalogChanged)
    Q_PROPERTY(QString bindCategory READ bindCategory WRITE setBindCategory NOTIFY filtersChanged)
    Q_PROPERTY(QVariantList bindRows READ bindRows NOTIFY rowsChanged)
    Q_PROPERTY(QStringList inputChoices READ inputChoices NOTIFY filtersChanged)
    Q_PROPERTY(QStringList activationModes READ activationModes NOTIFY catalogChanged)
    Q_PROPERTY(QVariantList optionDevices READ optionDevices NOTIFY catalogChanged)
    Q_PROPERTY(int optionDevice READ optionDevice WRITE setOptionDevice NOTIFY filtersChanged)
    Q_PROPERTY(QVariantList optionRows READ optionRows NOTIFY rowsChanged)
    Q_PROPERTY(QVariantList axisRows READ axisRows NOTIFY rowsChanged)

public:
    explicit GameSettingsController(QObject *parent = nullptr);

    bool active() const { return active_; }
    void setActive(bool active);
    QString status() const { return status_; }
    QString error() const { return error_; }
    QString profileDir() const { return profile_.dir; }
    QVariantList tabs() const;
    QString tab() const { return tab_; }
    void setTab(const QString &tab);
    QString search() const { return search_; }
    void setSearch(const QString &search);
    bool modifiedOnly() const { return modifiedOnly_; }
    void setModifiedOnly(bool on);
    QVariantList sections() const { return sections_; }
    int pendingCount() const { return pendingCount_; }
    int revision() const { return revision_; }
    bool gameRunning() const { return gameRunning_; }

    QVariantList bindDevices() const;
    QString bindDevice() const { return bindDevice_; }
    void setBindDevice(const QString &device);
    QStringList bindCategories() const;
    QString bindCategory() const { return bindCategory_; }
    void setBindCategory(const QString &category);
    QVariantList bindRows() const { return bindRows_; }
    QStringList inputChoices() const;
    QStringList activationModes() const;

    QVariantList optionDevices() const;
    int optionDevice() const { return optionDevice_; }
    void setOptionDevice(int index);
    QVariantList optionRows() const { return optionRows_; }
    QVariantList axisRows() const { return axisRows_; }

    Q_INVOKABLE QVariantMap settingState(const QString &name, int revision = 0) const;
    Q_INVOKABLE QVariantMap bindState(const QString &map, const QString &action, int revision = 0) const;
    Q_INVOKABLE QVariantMap optionState(const QString &group, int revision = 0) const;
    Q_INVOKABLE QVariantMap axisState(const QString &input, int revision = 0) const;
    Q_INVOKABLE void reload();
    Q_INVOKABLE void extractGameData();
    Q_INVOKABLE void setValue(const QString &name, const QString &value);
    Q_INVOKABLE void setToggle(const QString &name, bool checked);
    Q_INVOKABLE void resetValue(const QString &name);
    Q_INVOKABLE void resetTab();
    Q_INVOKABLE void save();
    Q_INVOKABLE void discard();
    Q_INVOKABLE void openProfileFolder();
    Q_INVOKABLE QVariantList backups() const;
    Q_INVOKABLE void restoreBackup(const QString &path);

    Q_INVOKABLE void setBinding(const QString &map, const QString &action, const QString &input,
                                const QString &activationMode, int multiTap);
    Q_INVOKABLE void unbind(const QString &map, const QString &action);
    Q_INVOKABLE void resetBinding(const QString &map, const QString &action);
    Q_INVOKABLE void resetDeviceBindings();
    Q_INVOKABLE QString keyName(int key, int nativeScanCode, int modifiers) const;
    Q_INVOKABLE QString mouseName(int button) const;
    Q_INVOKABLE QString describeInput(const QString &input) const;

    Q_INVOKABLE void setInvert(const QString &group, bool inverted);
    Q_INVOKABLE void setExponent(const QString &group, const QString &value);
    Q_INVOKABLE void resetOption(const QString &group);
    Q_INVOKABLE void setAxisOption(const QString &input, const QString &attribute, const QString &value);
    Q_INVOKABLE void resetAxisOption(const QString &input, const QString &attribute);

signals:
    void activeChanged();
    void statusChanged();
    void catalogChanged();
    void tabChanged();
    void filtersChanged();
    void rowsChanged();
    void pendingChanged();
    void revisionChanged();
    void gameRunningChanged();

private:
    struct Loaded;
    struct BindTarget
    {
        core::gamesettings::Device device = core::gamesettings::Device::Keyboard;
        int instance = 1;
    };
    struct OptionTarget
    {
        QString type;
        int instance = 1;
        QString product;
        QString label;
    };
    struct Effective
    {
        QString input;
        QString mode;
        QString defaultInput;
        QString defaultMode;
        int multiTap = 0;
        int defaultMultiTap = 0;
        bool modified = false;
    };

    AppController &app() const;
    void sourcesChanged();
    void build();
    void setStatus(const QString &status, const QString &error = {});
    void edited();
    void refreshRows();
    void refreshSettingRows();
    void refreshBindRows();
    void refreshOptionRows();
    void refreshGameRunning();
    QString sourceKey() const;
    BindTarget bindTarget() const;
    std::vector<OptionTarget> optionTargets() const;
    QString effectiveValue(const core::gamesettings::SettingDef &def) const;
    QVariantMap settingRow(const core::gamesettings::SettingDef &def) const;
    Effective effectiveBinding(const core::gamesettings::ActionDef &action) const;
    QString usageKey(const core::gamesettings::ActionDef &action, const Effective &e) const;
    void refreshBindUsage();
    QVariantMap bindRow(const core::gamesettings::ActionMapDef &map,
                        const core::gamesettings::ActionDef &action) const;
    QVariantMap optionRow(const OptionTarget &target, const core::gamesettings::OptionTree &tree,
                          int index) const;
    QVariantMap axisRow(const OptionTarget &target, const QString &axis) const;
    QString bindingText(core::gamesettings::Device device, const QString &input, const QString &mode,
                        int multiTap, const QString &defaultMode) const;
    bool writeFiles(QString *error);

    std::shared_ptr<const Loaded> loaded_;
    std::optional<core::gamesettings::AttributesFile> attributes_;
    std::optional<core::gamesettings::AttributesFile> savedAttributes_;
    std::optional<core::gamesettings::ActionMapsFile> actionMaps_;
    std::optional<core::gamesettings::ActionMapsFile> savedActionMaps_;
    core::gamesettings::ProfilePaths profile_;
    QVariantList sections_;
    QVariantList bindRows_;
    QVariantList optionRows_;
    QVariantList axisRows_;
    QHash<QString, QStringList> bindUsage_;
    QString status_ = QStringLiteral("idle");
    QString error_;
    QString tab_ = QStringLiteral("gameplay");
    QString search_;
    QString bindDevice_ = QStringLiteral("keyboard");
    QString bindCategory_;
    QString builtKey_;
    QTimer runningTimer_;
    int optionDevice_ = 0;
    int pendingCount_ = 0;
    int revision_ = 0;
    bool modifiedOnly_ = false;
    bool gameRunning_ = false;
    bool active_ = false;
    bool building_ = false;
    bool extractTried_ = false;
};
