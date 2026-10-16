#pragma once

#include <QHash>
#include <QList>
#include <QString>
#include <QStringList>

#include <memory>

namespace core::enh {

class RecordStore
{
public:
    static std::shared_ptr<const RecordStore> scan(const QString &recordsDir);

    const QString &recordsDir() const { return root_; }
    QString absolute(const QString &relative) const;
    bool dirExists(const QString &relative) const;
    qsizetype fileCount() const { return fileCount_; }

    QStringList indexRglob(const QString &relDir) const;
    QStringList filesIn(const QString &relDir) const;
    QStringList rglob(const QString &relDir) const;
    QStringList glob(const QString &relDir, const QString &pattern) const;

    static bool ntfsLess(const QString &a, const QString &b);

private:
    struct Dir
    {
        QStringList subdirs;
        QStringList files;
    };

    QStringList walkOrder(const QString &rel) const;

    QString root_;
    QHash<QString, Dir> dirs_;
    QStringList indexOrder_;
    qsizetype fileCount_ = 0;
};

}
