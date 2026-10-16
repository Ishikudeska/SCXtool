#include "core/log/Log.h"

#include "core/log/CrashHandler.h"

#include <QDir>
#include <QFileInfo>
#include <QLoggingCategory>

namespace core::log {

namespace {

QtMessageHandler g_previous = nullptr;
thread_local bool t_inHandler = false;

void handler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    if (!t_inHandler) {
        t_inHandler = true;
        LogHub::instance().record(type, QString::fromUtf8(context.category ? context.category : "default"),
                                  message);
        if (type == QtFatalMsg)
            crash::reportFatal(QStringLiteral("Fatal error: ") + message);
        t_inHandler = false;
    }
    if (g_previous)
        g_previous(type, context, message);
}

}

QString levelName(QtMsgType type)
{
    switch (type) {
    case QtDebugMsg:
        return QStringLiteral("DEBUG");
    case QtInfoMsg:
        return QStringLiteral("INFO");
    case QtWarningMsg:
        return QStringLiteral("WARNING");
    case QtCriticalMsg:
        return QStringLiteral("ERROR");
    case QtFatalMsg:
        return QStringLiteral("CRITICAL");
    }
    return QStringLiteral("INFO");
}

QString formatLine(const QDateTime &time, QtMsgType type, const QString &category, const QString &message)
{
    return QStringLiteral("%1 - %2 - %3 - %4")
        .arg(time.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss,zzz")), category, levelName(type), message);
}

LogHub &LogHub::instance()
{
    static LogHub *hub = new LogHub;
    return *hub;
}

void LogHub::install(const QString &logFile, bool debug)
{
    QLoggingCategory::setFilterRules(debug ? QStringLiteral("*.debug=true")
                                           : QStringLiteral("*.debug=false"));
    {
        QMutexLocker lock(&mutex_);
        file_.reset();
        if (!logFile.isEmpty()) {
            QDir().mkpath(QFileInfo(logFile).absolutePath());
            const QString previous = logFile + QStringLiteral(".previous");
            QFile::remove(previous);
            QFile::rename(logFile, previous);
            auto file = std::make_unique<QFile>(logFile);
            if (file->open(QIODevice::WriteOnly | QIODevice::Text))
                file_ = std::move(file);
        }
        if (installed_)
            return;
        installed_ = true;
    }
    g_previous = qInstallMessageHandler(handler);
}

bool LogHub::isInstalled() const
{
    QMutexLocker lock(&mutex_);
    return installed_;
}

QString LogHub::logFile() const
{
    QMutexLocker lock(&mutex_);
    return file_ ? file_->fileName() : QString();
}

QStringList LogHub::recentLines() const
{
    QMutexLocker lock(&mutex_);
    return QStringList(ring_.cbegin(), ring_.cend());
}

void LogHub::clearRecent()
{
    QMutexLocker lock(&mutex_);
    ring_.clear();
}

void LogHub::record(QtMsgType type, const QString &category, const QString &message)
{
    const QString line = formatLine(QDateTime::currentDateTime(), type, category, message);
    {
        QMutexLocker lock(&mutex_);
        ring_.push_back(line);
        while (qsizetype(ring_.size()) > kRingSize)
            ring_.pop_front();
        if (file_) {
            file_->write(line.toUtf8());
            file_->write("\n");
            file_->flush();
        }
    }
    emit lineLogged(line, int(type));
}

}
