#pragma once

#include "core/missions/Catalog.h"

#include <QObject>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>
#include <QtQml/qqmlregistration.h>

#include <memory>

class AppController;

class MissionsController : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(bool hasPlaces READ hasPlaces NOTIFY catalogChanged)
    Q_PROPERTY(int totalCount READ totalCount NOTIFY catalogChanged)
    Q_PROPERTY(QStringList categories READ categories NOTIFY catalogChanged)
    Q_PROPERTY(QStringList systems READ systems NOTIFY catalogChanged)
    Q_PROPERTY(QVariantList rows READ rows NOTIFY rowsChanged)
    Q_PROPERTY(QString search READ search WRITE setSearch NOTIFY filtersChanged)
    Q_PROPERTY(QString category READ category WRITE setCategory NOTIFY filtersChanged)
    Q_PROPERTY(QString system READ system WRITE setSystem NOTIFY filtersChanged)
    Q_PROPERTY(QString payout READ payout WRITE setPayout NOTIFY filtersChanged)
    Q_PROPERTY(QString sort READ sort WRITE setSort NOTIFY filtersChanged)
    Q_PROPERTY(bool blueprintsOnly READ blueprintsOnly WRITE setBlueprintsOnly NOTIFY filtersChanged)

public:
    explicit MissionsController(QObject *parent = nullptr);

    bool active() const { return active_; }
    void setActive(bool active);
    QString status() const { return status_; }
    bool hasPlaces() const { return catalog_ && catalog_->hasPlaces; }
    int totalCount() const { return catalog_ ? static_cast<int>(catalog_->missions.size()) : 0; }
    QStringList categories() const { return catalog_ ? catalog_->categories : QStringList(); }
    QStringList systems() const { return catalog_ ? catalog_->systems : QStringList(); }
    QVariantList rows() const { return rows_; }
    QString search() const { return filter_.search; }
    void setSearch(const QString &v);
    QString category() const { return filter_.category; }
    void setCategory(const QString &v);
    QString system() const { return filter_.system; }
    void setSystem(const QString &v);
    QString payout() const;
    void setPayout(const QString &v);
    QString sort() const;
    void setSort(const QString &v);
    bool blueprintsOnly() const { return filter_.blueprintsOnly; }
    void setBlueprintsOnly(bool v);

    Q_INVOKABLE QVariantMap details(int mission) const;
    Q_INVOKABLE void reload();
    Q_INVOKABLE void extractGameData();

signals:
    void activeChanged();
    void statusChanged();
    void catalogChanged();
    void rowsChanged();
    void filtersChanged();

private:
    AppController &app() const;
    QString sourceKey() const;
    void sourcesChanged();
    void build();
    void refilter();
    void setStatus(const QString &status);
    QString payoutText(const core::missions::Payout &payout) const;

    std::shared_ptr<const core::missions::Catalog> catalog_;
    core::missions::Filter filter_;
    QVariantList rows_;
    QString status_ = QStringLiteral("loading");
    QString builtKey_;
    bool active_ = false;
    bool building_ = false;
};
