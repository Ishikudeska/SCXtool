#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QtQml/qqmlregistration.h>

#include <deque>
#include <functional>

class PromptService : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("owned by App")

    Q_PROPERTY(QVariantMap current READ current NOTIFY currentChanged)
    Q_PROPERTY(bool active READ active NOTIFY currentChanged)

public:
    enum class Kind { Info, Warning, Error, Question };

    struct Prompt
    {
        Kind kind = Kind::Info;
        QString title;
        QString text;
        QString detail;
        QStringList buttons;
        int defaultButton = 0;
        QString checkbox;
        QStringList choices;
    };

    using Answer = std::function<void(int button, bool checked, int choice)>;

    explicit PromptService(QObject *parent = nullptr);

    void ask(Prompt prompt, Answer answer = {});
    void info(const QString &title, const QString &text, const QString &detail = {});
    void warning(const QString &title, const QString &text, const QString &detail = {});
    void error(const QString &title, const QString &text, const QString &detail = {});
    void confirm(const QString &title, const QString &text, std::function<void()> yes,
                 Kind kind = Kind::Question, std::function<void()> no = {});

    QVariantMap current() const { return current_; }
    bool active() const { return !current_.isEmpty(); }

    Q_INVOKABLE void answer(int button, bool checked = false, int choice = -1);

signals:
    void currentChanged();

private:
    void showNext();

    std::deque<std::pair<Prompt, Answer>> queue_;
    Answer pending_;
    QVariantMap current_;
};
