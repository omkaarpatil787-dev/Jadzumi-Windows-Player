#include <QtTest>
#include <QTemporaryDir>
#include <QFile>
#include <QSettings>
#include <QStandardPaths>
#include <QDateTime>
#include <limits>
#include <cmath>
#include "playback/Player.h"
#include "settings/Preferences.h"

class PlaybackTests : public QObject {
    Q_OBJECT
    QString library() const { return qEnvironmentVariable("JADZUMI_TEST_MPV"); }
    QString media() const { return qEnvironmentVariable("JADZUMI_TEST_MEDIA"); }
    QTemporaryDir settingsDirectory;
private slots:
    void initTestCase() {
        QStandardPaths::setTestModeEnabled(true);
        QCoreApplication::setOrganizationName("JadzumiTests");
        QCoreApplication::setApplicationName("Playback-"+QString::number(QDateTime::currentMSecsSinceEpoch()));
        QSettings::setDefaultFormat(QSettings::IniFormat);
        QSettings::setPath(QSettings::IniFormat,QSettings::UserScope,settingsDirectory.path());
    }
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
    void preferencesPersistAndRejectConflicts() {
        Preferences preferences;
        preferences.setTheme("amoled"); QCOMPARE(preferences.theme(),QString("amoled"));
        preferences.setTheme("neon"); QCOMPARE(preferences.theme(),QString("amoled"));
        preferences.setUiScale(99); QCOMPARE(preferences.uiScale(),1.25);
        preferences.setUiScale(std::numeric_limits<double>::quiet_NaN()); QCOMPARE(preferences.uiScale(),1.25);
        preferences.setAnimations(false); QVERIFY(preferences.effectiveReducedMotion());
        QVERIFY(!preferences.setShortcut("open","Space"));
        QCOMPARE(preferences.shortcuts().value("open").toString(),QString("Ctrl+O"));
        QVERIFY(!preferences.setShortcut("open","Alt+F4"));
        QVERIFY(preferences.setShortcut("playPause","Ctrl+P"));
        Preferences restored;
        QCOMPARE(restored.shortcuts().value("playPause").toString(),QString("Ctrl+P"));
        QCOMPARE(restored.theme(),QString("amoled"));
        restored.resetShortcuts();
        QCOMPARE(restored.shortcuts().value("playPause").toString(),QString("Space"));
    }
    void sqliteResumeSurvivesReopenAndChecksIdentity() {
        QTemporaryDir directory;
        const auto database=directory.filePath("history.sqlite");
        {
            HistoryStore history(database);
            QTRY_VERIFY_WITH_TIMEOUT(history.ready(),4000);
            history.save({{"path","C:/fixture.mp4"},{"title","Fixture"},{"position",5.25},{"duration",30.0},{"size",1234},{"modified",5678},{"played",1000}});
            QCOMPARE(history.recent().size(),1);
        }
        {
            HistoryStore history(database);
            QTRY_VERIFY_WITH_TIMEOUT(history.ready(),4000);
            QCOMPARE(history.resumePosition("C:/fixture.mp4",1234,5678),5.25);
            QCOMPARE(history.resumePosition("C:/fixture.mp4",1235,5678),0.0);
            QCOMPARE(history.resumePosition("C:/fixture.mp4",1234,5679),0.0);
            history.clear();
            QVERIFY(history.recent().isEmpty());
        }
        HistoryStore cleared(database);
        QTRY_VERIFY_WITH_TIMEOUT(cleared.ready(),4000);
        QVERIFY(cleared.recent().isEmpty());
    }
    void actualTracksChaptersAndAdvancedControls() {
        const QString fixture=QStringLiteral(JADZUMI_FIXTURE_DIR)+"/multitrack.mkv";
        QVERIFY(QFile::exists(fixture));
        Player player;player.setResumeEnabled(false);
        QVERIFY2(player.initialize(library(),0,Player::Output::HeadlessTest),qPrintable(player.error()));
        QVERIFY(player.open(QUrl::fromLocalFile(fixture),true));
        QTRY_VERIFY_WITH_TIMEOUT(player.loaded(),8000);
        QTRY_VERIFY(player.tracks().size()>=4);
        QTRY_COMPARE(player.chapters().size(),2);
        player.setSpeed(1.75);QTRY_COMPARE(player.speed(),1.75);
        player.setSpeed(1.0);QTRY_COMPARE(player.speed(),1.0);
        player.setAudioDelay(0.3);QTRY_VERIFY(std::abs(player.audioDelay()-0.3)<0.01);
        player.setSubtitleDelay(-0.5);QTRY_COMPARE(player.subtitleDelay(),-0.5);
        player.setSubtitleScale(1.4);QTRY_VERIFY(std::abs(player.subtitleScale()-1.4)<0.01);
        player.setSubtitlePosition(88);QTRY_COMPARE(player.subtitlePosition(),88.0);
        player.selectAudio(2);
        QTRY_COMPARE(player.diagnostics().value("audio-codec-name").toString(),QString("ac3"));
        QTRY_COMPARE(player.diagnostics().value("audio-params").toMap().value("channel-count").toInt(),6);
        player.selectAudio(1);
        QTRY_COMPARE(player.diagnostics().value("audio-codec-name").toString(),QString("aac"));
        player.toggleSubtitles();QTRY_VERIFY(!player.subtitlesVisible());
        player.toggleSubtitles();QTRY_VERIFY(player.subtitlesVisible());
        QTemporaryDir directory;
        const auto subtitlePath=directory.filePath("external.srt");
        QFile subtitle(subtitlePath);QVERIFY(subtitle.open(QIODevice::WriteOnly));
        subtitle.write("1\n00:00:00,000 --> 00:00:10,000\nExternal fixture subtitle\n");subtitle.close();
        QVERIFY(player.loadSubtitle(QUrl::fromLocalFile(subtitlePath)));
        auto subtitleCount=[&]{int count=0;for(const auto &track:player.tracks())if(track.toMap().value("type")=="sub")++count;return count;};
        QTRY_COMPARE(subtitleCount(),2);
        player.selectSubtitle(-1);
        auto selectedSubtitle=[&]{for(const auto &track:player.tracks())if(track.toMap().value("type")=="sub" && track.toMap().value("selected").toBool())return track.toMap().value("id").toInt();return -1;};
        QTRY_COMPARE(selectedSubtitle(),-1);
        player.selectSubtitle(1);QTRY_COMPARE(selectedSubtitle(),1);
        player.setAspect("4:3");QTRY_COMPARE(player.aspect(),QString("4:3"));
        player.setScaleMode("Fill");QTRY_COMPARE(player.scaleMode(),QString("Fill"));
        player.setScaleMode("Zoom");QTRY_COMPARE(player.scaleMode(),QString("Zoom"));
        player.setScaleMode("Fit");QTRY_COMPARE(player.scaleMode(),QString("Fit"));
        player.setRotation(90);QTRY_COMPARE(player.rotation(),90);
        player.setRotation(0);QTRY_COMPARE(player.rotation(),0);
        player.setLooping(true);QTRY_VERIFY(player.looping());
        player.setLooping(false);QTRY_VERIFY(!player.looping());
        player.setPaused(true);QTRY_VERIFY(player.paused());
        player.seek(2);QTRY_VERIFY(std::abs(player.position()-2)<0.2);
        player.setAbStart();QTRY_VERIFY(player.abStart().toDouble()>=1.8);
        player.seek(4);QTRY_VERIFY(std::abs(player.position()-4)<0.2);
        player.setAbEnd();QTRY_VERIFY(player.abEnd().toDouble()>=3.8);
        player.clearAb();QTRY_COMPARE(player.abStart(),QString("no"));QTRY_COMPARE(player.abEnd(),QString("no"));
        const double before=player.position();player.frameStep();QTRY_VERIFY(player.position()>before);
        player.setChapter(1);QTRY_COMPARE(player.chapterIndex(),1);
        QVERIFY2(player.error().isEmpty(),qPrintable(player.error()));
    }
    void realQueueAndAutoplayPreference() {
        QTemporaryDir directory;
        const auto second=directory.filePath("second.avi");
        QVERIFY(QFile::copy(media(),second));
        Player player;player.setResumeEnabled(false);player.setAutoplay(false);
        QVERIFY2(player.initialize(library(),0,Player::Output::HeadlessTest),qPrintable(player.error()));
        QVERIFY(player.openFiles({QUrl::fromLocalFile(media()),QUrl::fromLocalFile(second)}));
        QTRY_COMPARE(player.playlist().size(),2);
        QTRY_VERIFY(player.loaded());QTRY_COMPARE(player.playlistIndex(),0);
        player.seek(player.duration()-0.15);
        QTRY_COMPARE_WITH_TIMEOUT(player.playlistIndex(),1,5000);
        QTRY_VERIFY(player.loaded());QTRY_VERIFY(player.paused());
        player.previous();QTRY_COMPARE(player.playlistIndex(),0);
        player.playIndex(1);QTRY_COMPARE(player.playlistIndex(),1);
        player.playIndex(0);QTRY_COMPARE(player.playlistIndex(),0);
        player.next();QTRY_COMPARE(player.playlistIndex(),1);
        player.removeQueueItem(0);QTRY_COMPARE(player.playlist().size(),1);
        player.clearQueue();QTRY_COMPARE(player.playlist().size(),1);
        QVERIFY2(player.error().isEmpty(),qPrintable(player.error()));
    }
    void hevcMain10SoftwareDecode() {
        Player player;player.setResumeEnabled(false);
        QVERIFY2(player.initialize(library(),0,Player::Output::HeadlessTest),qPrintable(player.error()));
        QVERIFY(player.open(QUrl::fromLocalFile(QStringLiteral(JADZUMI_FIXTURE_DIR)+"/hevc-main10.mkv"),true));
        QTRY_VERIFY_WITH_TIMEOUT(player.loaded(),8000);
        QTRY_COMPARE(player.diagnostics().value("video-format").toString(),QString("hevc"));
        QTRY_VERIFY(QStringList({"yuv420p10","yuv420p10le"}).contains(player.diagnostics().value("video-dec-params").toMap().value("pixelformat").toString()));
        QVERIFY2(player.error().isEmpty(),qPrintable(player.error()));
    }
    void unicodeAndReservedFilenameCharacters() {
        QTemporaryDir directory;
        const auto name=directory.filePath(QString::fromUtf8("भारत ścieżka #1 %.avi"));
        QVERIFY(QFile::copy(media(),name));
        Player player;player.setResumeEnabled(false);
        QVERIFY2(player.initialize(library(),0,Player::Output::HeadlessTest),qPrintable(player.error()));
        QVERIFY(player.openPath(name,true));
        QTRY_VERIFY_WITH_TIMEOUT(player.loaded(),8000);
        QCOMPARE(QFileInfo(player.path()).canonicalFilePath(),QFileInfo(name).canonicalFilePath());
        QVERIFY2(player.error().isEmpty(),qPrintable(player.error()));
    }
};
QTEST_GUILESS_MAIN(PlaybackTests)
#include "PlaybackTests.moc"
