#pragma once

#include "core/tags/TagBuilder.h"

#include <QDateTime>
#include <QMap>
#include <QSet>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QVariant>

#include <memory>

namespace core {

QStringList channels();
inline const QString kDefaultChannel = QStringLiteral("LIVE");

inline const QString kDefaultLanguage = QStringLiteral("english");
QString scLanguageId(const QString &language);

class Settings;

inline const QStringList kMissionFieldKeys = {
    QStringLiteral("mission_type"),        QStringLiteral("difficulty"), QStringLiteral("spawns"),
    QStringLiteral("reputation"),          QStringLiteral("blueprints"), QStringLiteral("ace"),
    QStringLiteral("resource_signatures"),
};
inline const QStringList kMissionTitleTagKeys = {
    QStringLiteral("rep"), QStringLiteral("blueprint"), QStringLiteral("ace"),
    QStringLiteral("rs"),  QStringLiteral("rep_track"),
};

QString languageBaseUrl(const Settings &settings, const QString &language, const QString &languagesDir);

class Settings
{
public:
    Settings();
    explicit Settings(const QString &iniPath);

    static QString defaultFilePath();
    QString filePath() const;
    void sync();

    QVariant value(const QString &key, const QVariant &fallback = {}) const;
    void setValue(const QString &key, const QVariant &value);
    void remove(const QString &key);
    QStringList allKeys() const;

    QString scInstallRoot() const;
    void setScInstallRoot(const QString &path);
    QString activeChannel() const;
    void setActiveChannel(const QString &channel);

    QString selectedLanguage() const;
    void setSelectedLanguage(const QString &language);
    QString languageSourceOverride(const QString &language) const;
    void setLanguageSourceOverride(const QString &language, const QString &url);

    QString userDataDirOverride() const;
    void setUserDataDirOverride(const QString &path);
    QString cacheDirOverride() const;
    void setCacheDirOverride(const QString &path);

    QString favoritePrefix() const;
    void setFavoritePrefix(const QString &prefix);

    bool enhancementCategoryEnabled(const QString &categoryId) const;
    void setEnhancementCategoryEnabled(const QString &categoryId, bool enabled);
    QStringList enabledEnhancementFileIds() const;

    tags::TagConfig tagConfig(const QString &category);
    void setTagConfig(const QString &category, const tags::TagConfig &config);
    QMap<QString, tags::TagConfig> allTagConfigs();
    bool annotateMissionDescs() const;
    void setAnnotateMissionDescs(bool enabled);

    QString missionHeader(const QString &key) const;
    void setMissionHeader(const QString &key, const QString &text);
    QString repXpLabel() const;
    void setRepXpLabel(const QString &label);
    QString missionHeaderEmTag() const;
    void setMissionHeaderEmTag(const QString &tag);
    bool missionDetailField(const QString &field) const;
    void setMissionDetailField(const QString &field, bool enabled);
    bool missionTitleTag(const QString &field) const;
    void setMissionTitleTag(const QString &field, bool enabled);
    static bool missionTitleTagDefault(const QString &field);
    void migrateTitleTagSettings();
    bool statsPrepend() const;
    void setStatsPrepend(bool enabled);
    bool standardizeEarnableShipNames() const;
    void setStandardizeEarnableShipNames(bool enabled);
    bool rsOreNameAnnotations() const;
    void setRsOreNameAnnotations(bool enabled);

    QString theme() const;
    void setTheme(const QString &theme);
    QString uiMode() const;
    void setUiMode(const QString &mode);
    bool includeNewLines() const;
    void setIncludeNewLines(bool enabled);
    bool oneDriveWarningDismissed() const;
    void setOneDriveWarningDismissed(bool dismissed);
    QString tutorialCompletedVersion() const;
    void setTutorialCompletedVersion(const QString &version);
    bool tutorialDisabled() const;
    void setTutorialDisabled(bool disabled);

    QSet<QString> ownedItems() const;
    void setOwnedItems(const QSet<QString> &names);
    bool toggleOwnedItem(const QString &name);
    bool blueprintShowTags() const;
    void setBlueprintShowTags(bool enabled);
    bool scanOtherChannels() const;
    void setScanOtherChannels(bool enabled);
    QDateTime blueprintLogWatermark(const QString &channel = {}) const;
    void setBlueprintLogWatermark(const QDateTime &when, const QString &channel = {});

private:
    std::unique_ptr<QSettings> settings_;
};

}
