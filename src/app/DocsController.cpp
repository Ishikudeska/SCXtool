#include "DocsController.h"

#include <QCoreApplication>
#include <QFile>

DocsController::DocsController(QObject *parent) : QObject(parent)
{}

QString DocsController::markdown(const QString &name, const QString &language) const
{
    QStringList candidates;
    if (name == u"FAQ" || name == u"HELP")
        candidates << QStringLiteral(":/languages/%1/%2.md").arg(language, name);
    candidates << QStringLiteral(":/docs/english/%1.md").arg(name);
    for (const QString &path : std::as_const(candidates)) {
        QFile f(path);
        if (!f.open(QIODevice::ReadOnly))
            continue;
        QString text = QString::fromUtf8(f.readAll());
        const QString app = QCoreApplication::applicationName();
        text.replace(QStringLiteral("{app}"), app);
        return text;
    }
    return {};
}
