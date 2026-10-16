#pragma once

#include "core/net/AppUpdater.h"

#include <QObject>
#include <QtQml/qqmlregistration.h>

class AppController;

class UpdateController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("owned by App")

    Q_PROPERTY(bool enabled READ enabled CONSTANT)

public:
    explicit UpdateController(QObject *parent = nullptr);

    bool enabled() const;
    Q_INVOKABLE void check(bool interactive);

private:
    AppController &app() const;
    void onCheck(const core::net::UpdateCheck &check, const QString &current, bool interactive);
    void downloadAndInstall(const core::net::UpdateCheck &check);
};
