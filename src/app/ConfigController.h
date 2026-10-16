#pragma once

#include <QObject>
#include <QUrl>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>

class AppController;

class ConfigController : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString dataDir READ dataDir NOTIFY changed)
    Q_PROPERTY(bool dataDirOverridden READ dataDirOverridden NOTIFY changed)
    Q_PROPERTY(QString dataForgeBase READ dataForgeBase NOTIFY changed)
    Q_PROPERTY(bool dataForgeOverridden READ dataForgeOverridden NOTIFY changed)
    Q_PROPERTY(bool includeNewLines READ includeNewLines WRITE setIncludeNewLines NOTIFY changed)
    Q_PROPERTY(bool tutorialDisabled READ tutorialDisabled WRITE setTutorialDisabled NOTIFY changed)
    Q_PROPERTY(QVariantList importConflicts READ importConflicts NOTIFY importConflictsChanged)
    Q_PROPERTY(int importAdded READ importAdded NOTIFY importConflictsChanged)
    Q_PROPERTY(int importExcluded READ importExcluded NOTIFY importConflictsChanged)

public:
    explicit ConfigController(QObject *parent = nullptr);

    QString dataDir() const;
    bool dataDirOverridden() const;
    QString dataForgeBase() const;
    bool dataForgeOverridden() const;
    bool includeNewLines() const;
    void setIncludeNewLines(bool on);
    bool tutorialDisabled() const;
    void setTutorialDisabled(bool on);
    QVariantList importConflicts() const { return conflicts_; }
    int importAdded() const { return static_cast<int>(pendingAdd_.size()); }
    int importExcluded() const { return excluded_; }

    Q_INVOKABLE void setDataDir(const QString &path);
    Q_INVOKABLE void resetDataDir();
    Q_INVOKABLE void setDataForgeBase(const QString &path);
    Q_INVOKABLE void resetDataForgeBase();

    Q_INVOKABLE void resetUserIni();
    Q_INVOKABLE void restoreUserIni();
    Q_INVOKABLE void previewApply();

    Q_INVOKABLE void importIni(const QString &source);
    Q_INVOKABLE void finishImport(const QVariantList &resolutions);
    Q_INVOKABLE void cancelImport();

    Q_INVOKABLE QString defaultSettingsBackupPath() const;
    Q_INVOKABLE void exportSettings(const QUrl &target);
    Q_INVOKABLE void importSettings(const QUrl &source);

    Q_INVOKABLE QVariantList languageSources() const;
    Q_INVOKABLE void setLanguageSource(const QString &language, const QString &url);

signals:
    void changed();
    void importConflictsChanged();
    void importReady();

private:
    AppController &app() const;
    void importFromFile(const QString &file, const QString &tempToRemove);
    void writeImport(const QMap<QString, QString> &resolved, int resolvedCount);

    QVariantList conflicts_;
    QMap<QString, QString> pendingAdd_;
    QMap<QString, QString> current_;
    int excluded_ = 0;
};
