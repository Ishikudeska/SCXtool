#include "core/AppIdentity.h"
#include "core/Paths.h"
#include "core/ScInstall.h"
#include "core/Settings.h"
#include "core/enhancements/Generator.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QTest>

using namespace core;

namespace {

void touch(const QString &path, const QDateTime &mtime = {})
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly))
        qFatal("cannot open %s", qPrintable(f.fileName()));
    f.write("x");
    f.flush();
    if (mtime.isValid())
        f.setFileTime(mtime, QFileDevice::FileModificationTime);
}

PathRoots roots(const QTemporaryDir &dir, bool portable = false)
{
    PathRoots r;
    r.portable = portable;
    r.portableRoot = dir.filePath(QStringLiteral("exe/data"));
    r.documentsDir = dir.filePath(QStringLiteral("Documents"));
    r.localAppData = dir.filePath(QStringLiteral("Local"));
    return r;
}

}

class TestSettings : public QObject
{
    Q_OBJECT

private slots:
    void defaultsAndRoundTrip()
    {
        QTemporaryDir dir;
        const QString file = dir.filePath(QStringLiteral("settings.ini"));
        {
            Settings s(file);
            QCOMPARE(s.activeChannel(), QStringLiteral("LIVE"));
            QCOMPARE(s.selectedLanguage(), QStringLiteral("english"));
            QCOMPARE(s.favoritePrefix(), QStringLiteral("*"));
            QVERIFY(s.enhancementCategoryEnabled(QStringLiteral("missions")));
            QCOMPARE(s.enabledEnhancementFileIds().size(), 9);

            s.setActiveChannel(QStringLiteral("PTU"));
            s.setActiveChannel(QStringLiteral("NOT-A-CHANNEL"));
            s.setFavoritePrefix(QStringLiteral(" "));
            s.setEnhancementCategoryEnabled(QStringLiteral("ship_items"), false);
            s.setScInstallRoot(QStringLiteral("D:/Games/StarCitizen"));
            s.sync();
        }
        Settings s(file);
        QCOMPARE(s.activeChannel(), QStringLiteral("PTU"));
        QCOMPARE(s.favoritePrefix(), QStringLiteral(" "));
        QVERIFY(!s.enhancementCategoryEnabled(QStringLiteral("ship_items")));
        QVERIFY(!s.enabledEnhancementFileIds().contains(QStringLiteral("missile_enhancements")));
        QCOMPARE(s.enabledEnhancementFileIds().size(), 6);
        QCOMPARE(s.scInstallRoot(), QStringLiteral("D:/Games/StarCitizen"));

        s.setValue(QStringLiteral("tag_builder/components/config"), QStringLiteral("{\"a\":1}"));
        s.sync();
        QCOMPARE(Settings(file).value(QStringLiteral("tag_builder/components/config")).toString(),
                 QStringLiteral("{\"a\":1}"));
    }

    void tagConfigs()
    {
        QTemporaryDir dir;
        Settings s(dir.filePath(QStringLiteral("settings.ini")));
        QVERIFY(s.tagConfig(QStringLiteral("missiles")) == tags::defaultConfig(QStringLiteral("missiles")));
        QVERIFY(s.annotateMissionDescs());

        tags::TagConfig c = tags::defaultConfig(QStringLiteral("components"));
        c.separator = QStringLiteral("dot");
        s.setTagConfig(QStringLiteral("components"), c);
        QVERIFY(s.tagConfig(QStringLiteral("components")) == c);

        s.setValue(QStringLiteral("tag_builder/ship_weapons/config"), QStringLiteral("{broken"));
        QVERIFY(s.tagConfig(QStringLiteral("ship_weapons")) ==
                tags::defaultConfig(QStringLiteral("ship_weapons")));

        tags::TagConfig com = tags::defaultConfig(QStringLiteral("commodities"));
        com.separator = QStringLiteral("none");
        s.setTagConfig(QStringLiteral("commodities"), com);
        QCOMPARE(s.tagConfig(QStringLiteral("commodities")).separator, QStringLiteral("pipe"));
        s.setTagConfig(QStringLiteral("commodities"), com);
        QCOMPARE(s.tagConfig(QStringLiteral("commodities")).separator, QStringLiteral("none"));

        QCOMPARE(s.allTagConfigs().size(), tags::kCategories.size());
    }

    void generatorOptions()
    {
        QTemporaryDir dir;
        const QString file = dir.filePath(QStringLiteral("settings.ini"));
        {
            Settings s(file);
            QCOMPARE(s.repXpLabel(), QStringLiteral("Rep"));
            QCOMPARE(s.missionHeaderEmTag(), QStringLiteral("EM3"));
            for (const QString &f : kMissionFieldKeys)
                QVERIFY(s.missionDetailField(f));
            for (const QString &f : kMissionTitleTagKeys)
                QCOMPARE(s.missionTitleTag(f), f != u"rep_track");
            QVERIFY(!s.statsPrepend());
            QVERIFY(!s.standardizeEarnableShipNames());
            QVERIFY(s.rsOreNameAnnotations());

            const enh::GeneratorOptions defaults = enh::optionsFromSettings(s);
            QCOMPARE(defaults.categories->size(), enh::kGeneratorCategories.size());
            QVERIFY(!defaults.missionTitleTags.value(QStringLiteral("rep_track")));
            QCOMPARE(defaults.missionHeaders.value(QStringLiteral("items")), QStringLiteral("ITEM REWARDS"));

            s.setRepXpLabel(QStringLiteral("XP"));
            s.setMissionHeaderEmTag(QStringLiteral("EM1"));
            s.setMissionDetailField(QStringLiteral("spawns"), false);
            s.setMissionDetailField(QStringLiteral("nonsense"), false);
            s.setMissionTitleTag(QStringLiteral("rep_track"), true);
            s.setStatsPrepend(true);
            s.setRsOreNameAnnotations(false);
            s.setEnhancementCategoryEnabled(QStringLiteral("journal"), false);
            s.sync();
        }
        Settings s(file);
        QCOMPARE(s.missionHeaderEmTag(), QStringLiteral("EM3"));
        QVERIFY(!s.allKeys().contains(QStringLiteral("mission_field/nonsense")));
        const enh::GeneratorOptions o = enh::optionsFromSettings(s);
        QCOMPARE(o.repXpLabel, QStringLiteral("XP"));
        QVERIFY(!o.missionDetailFields.value(QStringLiteral("spawns")));
        QVERIFY(o.missionTitleTags.value(QStringLiteral("rep_track")));
        QVERIFY(o.statsPrepend);
        QVERIFY(!o.rsOreNameAnnotations);
        QVERIFY(!o.categories->contains(QStringLiteral("journal")));

        s.setValue(QStringLiteral("mission_field/blueprint_tag"), false);
        s.migrateTitleTagSettings();
        QVERIFY(!s.missionTitleTag(QStringLiteral("blueprint")));
        QVERIFY(s.missionTitleTag(QStringLiteral("ace")));
        s.setMissionTitleTag(QStringLiteral("blueprint"), true);
        s.migrateTitleTagSettings();
        QVERIFY(s.missionTitleTag(QStringLiteral("blueprint")));
    }

    void languageSources()
    {
        QTemporaryDir dir;
        Settings s(dir.filePath(QStringLiteral("settings.ini")));
        const QString shipped = QStringLiteral(SC_SOURCE_DIR "/resources/languages");
        QVERIFY(languageBaseUrl(s, QStringLiteral("french"), shipped).startsWith(QStringLiteral("https://")));
        QVERIFY(languageBaseUrl(s, QStringLiteral("english"), shipped).isEmpty());
        s.setLanguageSourceOverride(QStringLiteral("french"), QStringLiteral("https://example.com/fr.ini"));
        QCOMPARE(languageBaseUrl(s, QStringLiteral("french"), shipped),
                 QStringLiteral("https://example.com/fr.ini"));
        s.setLanguageSourceOverride(QStringLiteral("french"), QString());
        QVERIFY(s.languageSourceOverride(QStringLiteral("french")).isEmpty());
        QVERIFY(languageBaseUrl(s, QStringLiteral("french"), dir.filePath(QStringLiteral("none"))).isEmpty());
    }

    void unknownChannelFallsBackToLive()
    {
        QTemporaryDir dir;
        Settings s(dir.filePath(QStringLiteral("settings.ini")));
        s.setValue(QStringLiteral("active_channel"), QStringLiteral("BOGUS"));
        QCOMPARE(s.activeChannel(), QStringLiteral("LIVE"));
    }

    void languageIds()
    {
        QCOMPARE(scLanguageId(QStringLiteral("portuguese_br")), QStringLiteral("portuguese_(brazil)"));
        QCOMPARE(scLanguageId(QStringLiteral("german")), QStringLiteral("german_(germany)"));
        QCOMPARE(scLanguageId(QStringLiteral("klingon")), QStringLiteral("klingon"));
    }

    void pathLayout()
    {
        QTemporaryDir dir;
        Settings s(dir.filePath(QStringLiteral("settings.ini")));
        s.setScInstallRoot(dir.filePath(QStringLiteral("RSI/StarCitizen")));
        const Paths p(s, roots(dir));
        const QString app = QString::fromUtf8(core::identity::kAppName);
        const QString user = dir.filePath(QStringLiteral("Documents/") + app);

        QCOMPARE(p.userDataRoot(), user);
        QCOMPARE(p.channelDataDir(), user + QStringLiteral("/LIVE"));
        QCOMPARE(p.userIni(), user + QStringLiteral("/LIVE/user.ini"));
        QCOMPARE(p.backupsDir(), user + QStringLiteral("/LIVE/backups"));
        QCOMPARE(p.logsDir(), user + QStringLiteral("/logs"));
        QCOMPARE(p.baseIni(), user + QStringLiteral("/LIVE/cache/base.ini"));
        QCOMPARE(p.enhancementsDir(), user + QStringLiteral("/LIVE/cache"));
        QCOMPARE(p.baseIni(QStringLiteral("german")),
                 user + QStringLiteral("/LIVE/cache/lang/german/base.ini"));
        QCOMPARE(p.enhancementsDir(QStringLiteral("german")),
                 user + QStringLiteral("/LIVE/cache/lang/german"));
        QCOMPARE(p.dataForgeCacheDir(),
                 dir.filePath(QStringLiteral("Local/") + app + QStringLiteral("/LIVE/cache/dataforge")));
        QCOMPARE(p.p4kPath(), dir.filePath(QStringLiteral("RSI/StarCitizen/LIVE/Data.p4k")));
        QCOMPARE(p.gameGlobalIni(QStringLiteral("portuguese_br")),
                 dir.filePath(QStringLiteral(
                     "RSI/StarCitizen/LIVE/data/Localization/portuguese_(brazil)/global.ini")));

        s.setActiveChannel(QStringLiteral("PTU"));
        s.setSelectedLanguage(QStringLiteral("french"));
        QCOMPARE(p.baseIni(), user + QStringLiteral("/PTU/cache/lang/french/base.ini"));
        QCOMPARE(p.gameGlobalIni(), dir.filePath(QStringLiteral(
                                        "RSI/StarCitizen/PTU/data/Localization/french_(france)/global.ini")));
    }

    void overridesAndPortable()
    {
        QTemporaryDir dir;
        Settings s(dir.filePath(QStringLiteral("settings.ini")));
        s.setUserDataDirOverride(dir.filePath(QStringLiteral("Elsewhere")));
        s.setCacheDirOverride(dir.filePath(QStringLiteral("FastDisk")));
        const Paths p(s, roots(dir));
        QCOMPARE(p.userDataRoot(), dir.filePath(QStringLiteral("Elsewhere")));
        QCOMPARE(p.dataForgeCacheDir(), dir.filePath(QStringLiteral("FastDisk/LIVE/cache/dataforge")));
        QVERIFY(p.p4kPath().isEmpty());
        QVERIFY(p.gameGlobalIni().isEmpty());

        qputenv("SCX_TEST_DIR", dir.path().toUtf8());
        s.setUserDataDirOverride(QStringLiteral("%SCX_TEST_DIR%/expanded"));
        QCOMPARE(p.userDataRoot(), dir.filePath(QStringLiteral("expanded")));

        Settings portable(dir.filePath(QStringLiteral("p.ini")));
        const Paths pp(portable, roots(dir, true));
        QCOMPARE(pp.userDataRoot(), dir.filePath(QStringLiteral("exe/data")));
        QCOMPARE(pp.dataForgeCacheDir(), dir.filePath(QStringLiteral("exe/data/cache/LIVE/cache/dataforge")));
    }

    void installRoots()
    {
        QTemporaryDir dir;
        const QString root = dir.filePath(QStringLiteral("StarCitizen"));
        QVERIFY(!isScInstallRoot(root));
        QDir().mkpath(root + QStringLiteral("/LIVE"));
        QVERIFY(isScInstallRoot(root));
        QVERIFY(endsInChannel(root + QStringLiteral("/live")));
        QVERIFY(!endsInChannel(root));
        QCOMPARE(normalizeInstallRoot(root + QStringLiteral("/LIVE")), root);
        QCOMPARE(normalizeInstallRoot(QDir::toNativeSeparators(root)), root);
        QVERIFY(normalizeInstallRoot(dir.filePath(QStringLiteral("SmartCitizen 1.4.1"))).isEmpty());

        QVERIFY(installedChannels(root).isEmpty());
        touch(root + QStringLiteral("/PTU/Data.p4k"));
        touch(root + QStringLiteral("/LIVE/Data.p4k"));
        QCOMPARE(installedChannels(root), (QStringList{QStringLiteral("LIVE"), QStringLiteral("PTU")}));
    }

    void picksTheLiveInstall()
    {
        QTemporaryDir dir;
        const QString stale = dir.filePath(QStringLiteral("D/StarCitizen"));
        const QString live = dir.filePath(QStringLiteral("E/StarCitizen"));
        const QDateTime old(QDate(2025, 1, 1), QTime(0, 0));
        touch(stale + QStringLiteral("/LIVE/Data.p4k"), old);
        touch(live + QStringLiteral("/LIVE/Data.p4k"), old.addDays(10));
        touch(live + QStringLiteral("/PTU/Data.p4k"), old.addDays(400));
        QCOMPARE(pickLiveInstall({stale, live}), live);
        QCOMPARE(pickLiveInstall({live, stale}), live);
        QCOMPARE(pickLiveInstall({stale}), stale);
        QVERIFY(pickLiveInstall({}).isEmpty());
    }
};

QTEST_GUILESS_MAIN(TestSettings)
#include "tst_settings.moc"
