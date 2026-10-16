#pragma once

#include <QByteArray>
#include <QString>
#include <QUrl>

#include <atomic>

namespace core::net {

struct DownloadOptions
{
    int timeoutMs = 60'000;
    const std::atomic<bool> *cancel = nullptr;
};

struct DownloadResult
{
    enum class Status { Downloaded, Unchanged, Failed, Cancelled };
    Status status = Status::Failed;
    int httpStatus = 0;
    qint64 bytes = 0;
    QString error;

    bool ok() const { return status == Status::Downloaded || status == Status::Unchanged; }
};

QByteArray userAgent();

bool looksLikeHtml(const QByteArray &contentType, QByteArrayView body);

DownloadResult downloadIfChanged(const QUrl &url, const QString &dest, DownloadOptions options = {});

}
