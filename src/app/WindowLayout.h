#pragma once

#include <QObject>
#include <QVariantList>
#include <QVariantMap>
#include <QtQml/qqmlregistration.h>

class AppController;

class WindowLayout : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("owned by App")

public:
    explicit WindowLayout(QObject *parent = nullptr);

    Q_INVOKABLE QVariantMap windowGeometry() const;
    Q_INVOKABLE void saveWindowGeometry(const QVariantMap &geometry);
    Q_INVOKABLE QVariantList columnWidths() const;
    Q_INVOKABLE void saveColumnWidths(const QVariantList &widths);
    Q_INVOKABLE void resetWindowProportions();

signals:
    void windowProportionsReset();

private:
    AppController &app() const;
};
