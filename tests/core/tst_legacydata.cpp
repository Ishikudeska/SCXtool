#include "core/AppIdentity.h"
#include "core/LegacyData.h"
#include "core/Settings.h"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTest>

using namespace core;

namespace {

const QString kOld = QString::fromUtf8(identity::kLegacyAppName);
const QString kNew = QString::fromUtf8(identity::kAppName);

void touch(const QString &path, const QByteArray &bytes = "x")
{
    QDir().mkpath(QFileInfo(path).path());
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly))
        qFatal("cannot write %s", qPrintable(path));
    f.write(bytes);
}

QByteArray readAll(const QString &path)
{
    QFile f(path);
    return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
}

// A layout like Linux's: <config>/<org>/<app>/settings.ini.
LegacyDataLocations locationsIn(const QTemporaryDir &tmp)
{
    LegacyDataLocations where;
    where.settingsDirFrom = tmp.filePath(QStringLiteral("config/") + kOld + u'/' + kOld);
    where.settingsDirTo = tmp.filePath(QStringLiteral("config/") + kNew + u'/' + kNew);
    where.settingsFile = QDir(where.settingsDirTo).filePath(QStringLiteral("settings.ini"));
    where.roots.documentsDir = tmp.filePath(QStringLiteral("docs"));
    where.roots.localAppData = tmp.filePath(QStringLiteral("local"));
    return where;
}

} // namespace

class TestLegacyData : public QObject
{
    Q_OBJECT

private slots:
    void movesEveryFolderToTheNewName()
    {
        QTemporaryDir tmp;
        const LegacyDataLocations where = locationsIn(tmp);
        touch(QDir(where.settingsDirFrom).filePath(QStringLiteral("settings.ini")),
              "[General]\nui_mode=advanced\n");
        touch(tmp.filePath(QStringLiteral("docs/") + kOld + QStringLiteral("/LIVE/user.ini")), "edits");
        touch(tmp.filePath(QStringLiteral("local/") + kOld + QStringLiteral("/LIVE/cache/dataforge/x")));

        const QStringList log = migrateLegacyData(where);
        QCOMPARE(log.size(), 3);
        QVERIFY(readAll(where.settingsFile).contains("ui_mode=advanced"));
        QCOMPARE(readAll(tmp.filePath(QStringLiteral("docs/") + kNew + QStringLiteral("/LIVE/user.ini"))),
                 QByteArray("edits"));
        QVERIFY(QFileInfo::exists(
            tmp.filePath(QStringLiteral("local/") + kNew + QStringLiteral("/LIVE/cache/dataforge/x"))));
        QVERIFY(!QFileInfo::exists(tmp.filePath(QStringLiteral("docs/") + kOld)));
        QVERIFY(!QFileInfo::exists(tmp.filePath(QStringLiteral("local/") + kOld)));
        // The emptied <old> organization folder goes too.
        QVERIFY(!QFileInfo::exists(tmp.filePath(QStringLiteral("config/") + kOld)));
    }

    void repointsOverridesNamedAfterTheOldName()
    {
        QTemporaryDir tmp;
        const LegacyDataLocations where = locationsIn(tmp);
        const QString customData = tmp.filePath(QStringLiteral("elsewhere/") + kOld);
        const QString customCache = tmp.filePath(QStringLiteral("D-drive/MyCache"));
        touch(QDir(customData).filePath(QStringLiteral("LIVE/user.ini")));
        touch(QDir(customCache).filePath(QStringLiteral("x")));
        {
            QDir().mkpath(where.settingsDirFrom);
            Settings old(QDir(where.settingsDirFrom).filePath(QStringLiteral("settings.ini")));
            old.setUserDataDirOverride(customData);
            old.setCacheDirOverride(customCache);
            old.sync();
        }

        migrateLegacyData(where);
        Settings settings(where.settingsFile);
        const QString movedData = tmp.filePath(QStringLiteral("elsewhere/") + kNew);
        QCOMPARE(settings.userDataDirOverride(), movedData);
        QVERIFY(QFileInfo::exists(QDir(movedData).filePath(QStringLiteral("LIVE/user.ini"))));
        // A folder the user named themselves keeps its name.
        QCOMPARE(settings.cacheDirOverride(), customCache);
        QVERIFY(QFileInfo::exists(QDir(customCache).filePath(QStringLiteral("x"))));
    }

    void mergesIntoAnExistingFolderWithoutOverwriting()
    {
        QTemporaryDir tmp;
        const LegacyDataLocations where = locationsIn(tmp);
        const QString oldRoot = tmp.filePath(QStringLiteral("docs/") + kOld);
        const QString newRoot = tmp.filePath(QStringLiteral("docs/") + kNew);
        touch(oldRoot + QStringLiteral("/LIVE/user.ini"), "old");
        touch(oldRoot + QStringLiteral("/LIVE/backups/user.ini.bak_1"), "backup");
        touch(oldRoot + QStringLiteral("/PTU/user.ini"), "ptu");
        touch(newRoot + QStringLiteral("/LIVE/user.ini"), "new");
        touch(newRoot + QStringLiteral("/logs/app.log"), "log");

        QCOMPARE(migrateLegacyData(where).size(), 1);
        QCOMPARE(readAll(newRoot + QStringLiteral("/LIVE/user.ini")), QByteArray("new"));
        QCOMPARE(readAll(newRoot + QStringLiteral("/LIVE/backups/user.ini.bak_1")), QByteArray("backup"));
        QCOMPARE(readAll(newRoot + QStringLiteral("/PTU/user.ini")), QByteArray("ptu"));
        QCOMPARE(readAll(newRoot + QStringLiteral("/logs/app.log")), QByteArray("log"));
        // The one clash stays behind; everything else has gone.
        QCOMPARE(readAll(oldRoot + QStringLiteral("/LIVE/user.ini")), QByteArray("old"));
        QVERIFY(!QFileInfo::exists(oldRoot + QStringLiteral("/LIVE/backups")));
        QVERIFY(!QFileInfo::exists(oldRoot + QStringLiteral("/PTU")));
    }

    void mergesAwayAFolderOnlyTheLogsWereWrittenTo()
    {
        QTemporaryDir tmp;
        const LegacyDataLocations where = locationsIn(tmp);
        const QString oldRoot = tmp.filePath(QStringLiteral("docs/") + kOld);
        const QString newRoot = tmp.filePath(QStringLiteral("docs/") + kNew);
        touch(oldRoot + QStringLiteral("/LIVE/user.ini"), "edits");
        touch(oldRoot + QStringLiteral("/logs/app.log"), "old log");
        touch(newRoot + QStringLiteral("/logs/app.log"), "smoke test");

        migrateLegacyData(where);
        QCOMPARE(readAll(newRoot + QStringLiteral("/LIVE/user.ini")), QByteArray("edits"));
        QCOMPARE(readAll(newRoot + QStringLiteral("/logs/app.log")), QByteArray("smoke test"));
        QVERIFY(QFileInfo::exists(oldRoot + QStringLiteral("/logs/app.log")));
        QVERIFY(!QFileInfo::exists(oldRoot + QStringLiteral("/LIVE")));
    }

    void skipsPortableBuilds()
    {
        QTemporaryDir tmp;
        LegacyDataLocations where = locationsIn(tmp);
        where.roots.portable = true;
        touch(tmp.filePath(QStringLiteral("docs/") + kOld + QStringLiteral("/LIVE/user.ini")));

        QVERIFY(migrateLegacyData(where).isEmpty());
        QVERIFY(QFileInfo::exists(tmp.filePath(QStringLiteral("docs/") + kOld)));
    }
};

QTEST_GUILESS_MAIN(TestLegacyData)
#include "tst_legacydata.moc"
