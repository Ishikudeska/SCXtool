#pragma once

#include <QString>
#include <QStringList>

#include <span>

namespace core::enhancements {

struct File
{
    const char *id;
    const char *fileName;
};

struct Category
{
    const char *id;
    QString label;
    QStringList fileIds;
};

std::span<const File> files();
std::span<const Category> categories();

QString categoryLabelForFile(const QString &fileId);
QString fileNameFor(const QString &fileId);

}
