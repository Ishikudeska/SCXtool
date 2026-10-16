#pragma once

#include <QObject>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>

class TutorialController : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(bool running READ running NOTIFY changed)
    Q_PROPERTY(int index READ index NOTIFY changed)
    Q_PROPERTY(int count READ count NOTIFY changed)
    Q_PROPERTY(QVariantMap step READ step NOTIFY changed)

public:
    explicit TutorialController(QObject *parent = nullptr);

    bool running() const { return running_; }
    int index() const { return index_; }
    int count() const { return static_cast<int>(steps_.size()); }
    QVariantMap step() const;

    Q_INVOKABLE bool shouldAutoStart() const;
    Q_INVOKABLE void start();
    Q_INVOKABLE void next();
    Q_INVOKABLE void back();
    Q_INVOKABLE void skip();

signals:
    void changed();
    void finished(bool completed);

private:
    void load();
    void finish(bool completed);

    QVariantList steps_;
    int index_ = 0;
    bool running_ = false;
};
