#include "../syncthingconfig.h"
#include "../syncthingconnection.h"
#include "../syncthingconnectionsettings.h"
#include "../syncthingservice.h"
#include "../utils.h"

#include <c++utilities/chrono/datetime.h>
#include <c++utilities/chrono/format.h>
#include <c++utilities/chrono/timespan.h>
#include <c++utilities/tests/testutils.h>

#include "../testhelper/helper.h"

#include <cppunit/TestFixture.h>

#include <QFile>
#include <QUrl>

using namespace std;
using namespace Data;
using namespace ChronoUtilities;
using namespace TestUtilities;

using namespace CPPUNIT_NS;

/*!
 * \brief The MiscTests class tests various features of the connector library.
 */
class MiscTests : public TestFixture {
    CPPUNIT_TEST_SUITE(MiscTests);
    CPPUNIT_TEST(testParsingConfig);
    CPPUNIT_TEST(testUtils);
    CPPUNIT_TEST(testService);
    CPPUNIT_TEST(testConnectionSettingsAndLoadingSelfSignedCert);
    CPPUNIT_TEST(testSyncthingDir);
    CPPUNIT_TEST_SUITE_END();

public:
    MiscTests();

    void testParsingConfig();
    void testUtils();
    void testService();
    void testConnectionSettingsAndLoadingSelfSignedCert();
    void testSyncthingDir();

    void setUp();
    void tearDown();

private:
};

CPPUNIT_TEST_SUITE_REGISTRATION(MiscTests);

MiscTests::MiscTests()
{
}

//
// test setup
//

void MiscTests::setUp()
{
}

void MiscTests::tearDown()
{
}

//
// actual test
//

/*!
 * \brief Tests basic behaviour of the SyncthingConnection class.
 */
void MiscTests::testParsingConfig()
{
    SyncthingConfig config;
    CPPUNIT_ASSERT(!config.restore(QStringLiteral("non-existant-file")));
    CPPUNIT_ASSERT(config.restore(QString::fromLocal8Bit(testFilePath("testconfig/config.xml").data())));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("address", QStringLiteral("127.0.0.1:4001"), config.guiAddress);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("API key", QStringLiteral("syncthingconnectortest"), config.guiApiKey);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("user", QStringLiteral("nobody"), config.guiUser);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("password", QStringLiteral("$2a$12$35MnbsQgQNn1hzPYK/lWXOaP.U5D2TO0nuuQy2M4gsqJB4ff4q2RK"), config.guiPasswordHash);
    CPPUNIT_ASSERT_MESSAGE("TLS", !config.guiEnforcesSecureConnection);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("url", QStringLiteral("http://127.0.0.1:4001"), config.syncthingUrl());
    config.guiEnforcesSecureConnection = true;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("url", QStringLiteral("https://127.0.0.1:4001"), config.syncthingUrl());
    const QString configFile(SyncthingConfig::locateConfigFile());
    CPPUNIT_ASSERT(configFile.isEmpty() || QFile::exists(configFile));
    const QString httpsCert(SyncthingConfig::locateHttpsCertificate());
    CPPUNIT_ASSERT(httpsCert.isEmpty() || QFile::exists(httpsCert));
}

/*!
 * \brief Tests utils.
 */
void MiscTests::testUtils()
{
    CPPUNIT_ASSERT_EQUAL(QStringLiteral("right now"), agoString(DateTime::now()));
    CPPUNIT_ASSERT_EQUAL(QStringLiteral("5 h ago"), agoString(DateTime::now() - TimeSpan::fromHours(5.0)));
    CPPUNIT_ASSERT(isLocal(QUrl(QStringLiteral("http://127.0.0.1"))));
    CPPUNIT_ASSERT(isLocal(QUrl(QStringLiteral("http://[::1]"))));
    CPPUNIT_ASSERT(isLocal(QUrl(QStringLiteral("http://localhost/"))));
    CPPUNIT_ASSERT(!isLocal(QUrl(QStringLiteral("http://157.3.52.34"))));
}

/*!
 * \brief Tests SyncthingService class, but error case with non-existant service so far.
 */
void MiscTests::testService()
{
    SyncthingService service;
    service.isSystemdAvailable();
    service.setUnitName(QStringLiteral("non-existant.service"));
    CPPUNIT_ASSERT(!service.isUnitAvailable());
    CPPUNIT_ASSERT_EQUAL(QString(), service.description());
    CPPUNIT_ASSERT(!service.isRunning());
    CPPUNIT_ASSERT(!service.isEnabled());
    service.toggleRunning();
    service.setEnabled(true);
}

void MiscTests::testConnectionSettingsAndLoadingSelfSignedCert()
{
    SyncthingConnectionSettings settings;
    settings.syncthingUrl = QStringLiteral("http://localhost:8080");
    settings.apiKey = QByteArray("foo");
    settings.httpsCertPath = SyncthingConfig::locateHttpsCertificate();
    if (!settings.httpsCertPath.isEmpty() && settings.loadHttpsCert()) {
        CPPUNIT_ASSERT_EQUAL(4, settings.expectedSslErrors.size());
    } else {
        CPPUNIT_ASSERT_EQUAL(0, settings.expectedSslErrors.size());
    }
    SyncthingConnection connection;
    CPPUNIT_ASSERT(connection.applySettings(settings));
    CPPUNIT_ASSERT(!connection.loadSelfSignedCertificate());
    settings.syncthingUrl = QStringLiteral("https://localhost:8080");
    CPPUNIT_ASSERT(connection.applySettings(settings));
    connection.m_configDir = QStringLiteral("some-non/existent-dir");
    bool errorOccured = false;
    const function<void(const QString &)> errorHandler
        = [&errorOccured](const QString &message) { errorOccured |= message == QStringLiteral("Unable to load certificate used by Syncthing."); };
    waitForSignals(bind(&SyncthingConnection::loadSelfSignedCertificate, &connection), 1,
        signalInfo(&connection, &SyncthingConnection::error, errorHandler, &errorOccured));
    settings.expectedSslErrors.clear();
    CPPUNIT_ASSERT(!connection.applySettings(settings));
}

void MiscTests::testSyncthingDir()
{
    SyncthingDir dir;
    dir.deviceIds << QStringLiteral("dev1") << QStringLiteral("dev2");

    DateTime updateTime(DateTime::fromDate(2005, 2, 3));
    CPPUNIT_ASSERT(dir.assignStatus(SyncthingDirStatus::Unshared, updateTime));
    CPPUNIT_ASSERT_EQUAL(QStringLiteral("unshared"), dir.statusString());
    CPPUNIT_ASSERT_EQUAL(updateTime, dir.lastStatusUpdate);

    CPPUNIT_ASSERT(!dir.assignStatus(SyncthingDirStatus::Scanning, DateTime::fromDate(2003, 6, 7)));
    CPPUNIT_ASSERT_EQUAL(updateTime, dir.lastStatusUpdate);
    CPPUNIT_ASSERT_EQUAL(QStringLiteral("unshared"), dir.statusString());

    const DateTime lastScanTime(DateTime::now());
    updateTime += TimeSpan::fromSeconds(5);
    CPPUNIT_ASSERT(dir.assignStatus(SyncthingDirStatus::Scanning, updateTime));
    CPPUNIT_ASSERT(dir.lastScanTime.isNull());
    CPPUNIT_ASSERT_EQUAL(QStringLiteral("scanning"), dir.statusString());

    CPPUNIT_ASSERT(dir.assignStatus(SyncthingDirStatus::Idle, updateTime += TimeSpan::fromSeconds(2)));
    CPPUNIT_ASSERT_EQUAL(updateTime, dir.lastStatusUpdate);
    CPPUNIT_ASSERT(dir.lastScanTime >= lastScanTime);

    dir.itemErrors.emplace_back(QStringLiteral("message"), QStringLiteral("path"));
    CPPUNIT_ASSERT(dir.assignStatus(SyncthingDirStatus::Idle, updateTime += TimeSpan::fromMinutes(1.5)));
    CPPUNIT_ASSERT_EQUAL(QStringLiteral("out of sync"), dir.statusString());

    CPPUNIT_ASSERT(!dir.assignStatus(SyncthingDirStatus::Idle, updateTime += TimeSpan::fromMinutes(1.5)));
    CPPUNIT_ASSERT(dir.assignStatus(QStringLiteral("syncing"), updateTime += TimeSpan::fromMinutes(1.5)));

    dir.itemErrors.clear();
    CPPUNIT_ASSERT(dir.assignStatus(QStringLiteral("error"), updateTime += TimeSpan::fromMinutes(1.5)));
    CPPUNIT_ASSERT_EQUAL(QStringLiteral("out of sync"), dir.statusString());

    CPPUNIT_ASSERT(dir.assignStatus(QStringLiteral("wrong status"), updateTime += TimeSpan::fromMinutes(1.5)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong status treated as idle", QStringLiteral("idle"), dir.statusString());

    CPPUNIT_ASSERT_MESSAGE("older status discarded", !dir.assignStatus(QStringLiteral("scanning"), updateTime - TimeSpan::fromSeconds(1)));
    CPPUNIT_ASSERT_EQUAL(QStringLiteral("idle"), dir.statusString());

    dir.itemErrors.emplace_back(QStringLiteral("message"), QStringLiteral("path"));
    CPPUNIT_ASSERT(dir.assignStatus(QStringLiteral("idle"), updateTime += TimeSpan::fromMinutes(1.5)));
    CPPUNIT_ASSERT_EQUAL(QStringLiteral("out of sync"), dir.statusString());

    dir.itemErrors.clear();
    dir.deviceIds.clear();
    CPPUNIT_ASSERT(dir.assignStatus(QStringLiteral("idle"), updateTime += TimeSpan::fromMinutes(1.5)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("dir considered unshared when no devs present", QStringLiteral("unshared"), dir.statusString());
    CPPUNIT_ASSERT(!dir.assignStatus(SyncthingDirStatus::Idle, updateTime += TimeSpan::fromMinutes(1.5)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("dir considered unshared when no devs present", QStringLiteral("unshared"), dir.statusString());

    updateTime += TimeSpan::fromMinutes(1.5);
    CPPUNIT_ASSERT_MESSAGE("same status again not considered an update", !dir.assignStatus(QStringLiteral("idle"), updateTime));
}
