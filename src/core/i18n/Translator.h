#pragma once

#include <QHash>
#include <QJsonObject>
#include <QString>
#include <QStringList>
#include <QTranslator>
#include <QVariantHash>

#include <atomic>
#include <memory>

namespace core::i18n {

inline const QString kDefaultLanguage = QStringLiteral("english");

class Catalog
{
public:
    Catalog() = default;

    static Catalog load(const QString &languagesDir, const QString &language);
    static Catalog fromTrees(const QString &language, const QJsonObject &english,
                             const QJsonObject &overlay = {});

    const QString &language() const { return language_; }
    bool isEmpty() const { return strings_.isEmpty(); }
    QString lookup(const QString &key) const { return strings_.value(key); }

private:
    QString language_ = kDefaultLanguage;
    QHash<QString, QString> strings_;
};

void deepMerge(QJsonObject &base, const QJsonObject &overlay);

class JsonTranslator : public QTranslator
{
    Q_OBJECT

public:
    explicit JsonTranslator(QObject *parent = nullptr);

    void setCatalog(std::shared_ptr<const Catalog> catalog);
    std::shared_ptr<const Catalog> catalog() const;

    QString translate(const char *context, const char *sourceText, const char *disambiguation = nullptr,
                      int n = -1) const override;
    bool isEmpty() const override;

private:
    std::atomic<std::shared_ptr<const Catalog>> catalog_;
};

QString format(const QString &text, const QVariantHash &args);

QString tr(const char *key, const QVariantHash &args = {});

QStringList availableLanguages(const QString &languagesDir);

}
