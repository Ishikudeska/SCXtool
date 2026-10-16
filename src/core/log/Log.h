#pragma once

#include <QDateTime>
#include <QFile>
#include <QMutex>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QtLogging>

#include <deque>
#include <memory>

namespace core::log {

inline constexpr qsizetype kRingSize = 5000;

QString levelName(QtMsgType type);
QString formatLine(const QDateTime &time, QtMsgType type, const QString &category, const QString &message);

class LogHub : public QObject
{
    Q_OBJECT

public:
    static LogHub &instance();

    void install(const QString &logFile = {}, bool debug = false);
    bool isInstalled() const;
    QString logFile() const;

    QStringList recentLines() const;
    void clearRecent();

    void record(QtMsgType type, const QString &category, const QString &message);

signals:
    void lineLogged(const QString &line, int type);

private:
    explicit LogHub(QObject *parent = nullptr) : QObject(parent) {}

    mutable QMutex mutex_;
    std::deque<QString> ring_;
    std::unique_ptr<QFile> file_;
    bool installed_ = false;
};

}
