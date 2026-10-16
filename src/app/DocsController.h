#pragma once

#include <QObject>
#include <QString>
#include <QtQml/qqmlregistration.h>

class DocsController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    explicit DocsController(QObject *parent = nullptr);

    Q_INVOKABLE QString markdown(const QString &name, const QString &language) const;
};
