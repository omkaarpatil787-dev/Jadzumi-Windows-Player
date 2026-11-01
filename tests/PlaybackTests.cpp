#include <QtTest>
#include <QTemporaryDir>
#include <QFile>
#include <cmath>
#include "playback/Player.h"

class PlaybackTests : public QObject {
    Q_OBJECT
    QString library() const { return qEnvironmentVariable("JADZUMI_TEST_MPV"); }
    QString media() const { return qEnvironmentVariable("JADZUMI_TEST_MEDIA"); }
private slots:
    void missingLibraryIsVisible() {
        Player player;
        QVERIFY(!player.initialize("/nonexistent/jadzumi-mpv.dll", 0, Player::Output::HeadlessTest));
        QVERIFY(!player.ready());
        QVERIFY(!player.error().isEmpty());
        QVERIFY(!player.open(QUrl("https://example.org/video.mp4")));
    }
    void realPlaybackContract() {
        QVERIFY2(!library().isEmpty() && !media().isEmpty(),
                 "Set JADZUMI_TEST_MPV and JADZUMI_TEST_MEDIA to run real integration tests.");
        Player player;
        QVERIFY2(player.initialize(library(), 0, Player::Output::HeadlessTest), qPrintable(player.error()));
        QVERIFY(!player.open(QUrl("https://example.org/movie.mp4")));
        QVERIFY(!player.open(QUrl("file://server/share/movie.mp4")));
        QSignalSpy loaded(&player, &Player::fileLoaded);
        QVERIFY(player.open(QUrl::fromLocalFile(media())));
        QTRY_COMPARE_WITH_TIMEOUT(loaded.count(), 1, 8000);
        QTRY_VERIFY_WITH_TIMEOUT(player.duration() >= 10, 4000);
        QTRY_VERIFY_WITH_TIMEOUT(player.position() > 0.1, 4000);
        QTRY_VERIFY_WITH_TIMEOUT(!player.diagnostics().value("video-codec").toString().isEmpty(), 4000);
        QVERIFY(player.error().isEmpty());
        qInfo().noquote() << "Observed backend:" << player.diagnostics();
        player.setPaused(true);
        QTRY_VERIFY(player.paused());
        const auto pausedPosition = player.position();
        QTest::qWait(200);
        QVERIFY(std::abs(player.position() - pausedPosition) < 0.15);
        player.seek(5.0);
        QTRY_VERIFY_WITH_TIMEOUT(std::abs(player.position() - 5) < 0.3, 4000);
        player.setVolume(37);
        QTRY_COMPARE(player.volume(), 37.0);
        player.toggleMute(); QTRY_VERIFY(player.muted());
        player.toggleMute(); QTRY_VERIFY(!player.muted());
        player.setVolume(150); QTRY_COMPARE(player.volume(), 100.0);
        player.setPaused(false); QTRY_VERIFY(!player.paused());
        player.seekRelative(2);
        QTRY_VERIFY_WITH_TIMEOUT(player.position() >= 6.8, 4000);
        QVERIFY(player.error().isEmpty());
        player.stop();
        QTRY_VERIFY(!player.loaded());
        QTRY_COMPARE(player.duration(), 0.0);
        QVERIFY(player.open(QUrl::fromLocalFile(media())));
        QTRY_COMPARE_WITH_TIMEOUT(loaded.count(), 2, 8000);
        player.shutdown();
        QVERIFY(!player.ready());
        QTest::qWait(30); // Flush any callback queued during teardown.
        QVERIFY(player.error().isEmpty());
    }
    void corruptMediaFailsAndRecovers() {
        QVERIFY(!library().isEmpty() && !media().isEmpty());
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const auto corruptPath = directory.filePath("broken.mp4");
        QFile file(corruptPath); QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("not a media container"); file.close();
        Player player;
        QVERIFY2(player.initialize(library(), 0, Player::Output::HeadlessTest), qPrintable(player.error()));
        QVERIFY(player.open(QUrl::fromLocalFile(corruptPath)));
        QTRY_VERIFY_WITH_TIMEOUT(!player.error().isEmpty(), 8000);
        QVERIFY(!player.loaded());
        QVERIFY(player.open(QUrl::fromLocalFile(media())));
        QTRY_VERIFY_WITH_TIMEOUT(player.loaded(), 8000);
        QVERIFY(player.error().isEmpty());
    }
    void callbackTeardownStress() {
        QVERIFY(!library().isEmpty() && !media().isEmpty());
        for (int i = 0; i < 12; ++i) {
            Player player;
            QVERIFY2(player.initialize(library(), 0, Player::Output::HeadlessTest), qPrintable(player.error()));
            QVERIFY(player.open(QUrl::fromLocalFile(media())));
            player.setPaused(true);
            player.shutdown();
            QCoreApplication::processEvents();
            QVERIFY(!player.ready());
        }
    }
};
QTEST_GUILESS_MAIN(PlaybackTests)
#include "PlaybackTests.moc"
