#pragma once
#include <QObject>
#include <QUrl>
#include <QVariantMap>
#include <QVariantList>
#include <QSet>
#include <atomic>
#include <memory>
#include "library/HistoryStore.h"

class MpvApi;
struct mpv_handle;
class Player final : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool ready READ ready NOTIFY readyChanged)
    Q_PROPERTY(bool paused READ paused NOTIFY stateChanged)
    Q_PROPERTY(bool loaded READ loaded NOTIFY stateChanged)
    Q_PROPERTY(bool muted READ muted NOTIFY stateChanged)
    Q_PROPERTY(double position READ position NOTIFY stateChanged)
    Q_PROPERTY(double duration READ duration NOTIFY stateChanged)
    Q_PROPERTY(double volume READ volume NOTIFY stateChanged)
    Q_PROPERTY(double speed READ speed NOTIFY controlsChanged)
    Q_PROPERTY(double audioDelay READ audioDelay NOTIFY controlsChanged)
    Q_PROPERTY(double subtitleDelay READ subtitleDelay NOTIFY controlsChanged)
    Q_PROPERTY(double subtitleScale READ subtitleScale NOTIFY controlsChanged)
    Q_PROPERTY(double subtitlePosition READ subtitlePosition NOTIFY controlsChanged)
    Q_PROPERTY(bool subtitlesVisible READ subtitlesVisible NOTIFY controlsChanged)
    Q_PROPERTY(QString title READ title NOTIFY stateChanged)
    Q_PROPERTY(QString path READ path NOTIFY stateChanged)
    Q_PROPERTY(QVariantList tracks READ tracks NOTIFY tracksChanged)
    Q_PROPERTY(QVariantList chapters READ chapters NOTIFY chaptersChanged)
    Q_PROPERTY(QVariantList playlist READ playlist NOTIFY queueChanged)
    Q_PROPERTY(int playlistIndex READ playlistIndex NOTIFY queueChanged)
    Q_PROPERTY(int chapterIndex READ chapterIndex NOTIFY chaptersChanged)
    Q_PROPERTY(bool looping READ looping NOTIFY controlsChanged)
    Q_PROPERTY(QString abStart READ abStart NOTIFY controlsChanged)
    Q_PROPERTY(QString abEnd READ abEnd NOTIFY controlsChanged)
    Q_PROPERTY(QString aspect READ aspect NOTIFY controlsChanged)
    Q_PROPERTY(QString scaleMode READ scaleMode NOTIFY controlsChanged)
    Q_PROPERTY(int rotation READ rotation NOTIFY controlsChanged)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)
    Q_PROPERTY(QVariantMap diagnostics READ diagnostics NOTIFY diagnosticsChanged)
    Q_PROPERTY(HistoryStore* history READ history CONSTANT)
public:
    enum class Output { WindowsD3D11, HeadlessTest };
    explicit Player(QObject *parent=nullptr);
    ~Player() override;
    bool initialize(const QString &libraryPath, quintptr windowId, Output output);
    void shutdown();
    bool ready() const { return m_handle!=nullptr; }
    bool loaded() const { return m_loaded; }
    bool paused() const { return value("pause",true).toBool(); }
    bool muted() const { return value("mute",false).toBool(); }
    double position() const { return value("time-pos",0.0).toDouble(); }
    double duration() const { return value("duration",0.0).toDouble(); }
    double volume() const { return value("volume",100.0).toDouble(); }
    double speed() const { return value("speed",1.0).toDouble(); }
    double audioDelay() const { return value("audio-delay",0.0).toDouble(); }
    double subtitleDelay() const { return value("sub-delay",0.0).toDouble(); }
    double subtitleScale() const { return value("sub-scale",1.0).toDouble(); }
    double subtitlePosition() const { return value("sub-pos",100.0).toDouble(); }
    bool subtitlesVisible() const { return value("sub-visibility",true).toBool(); }
    QString title() const { return value("media-title").toString(); }
    QString path() const { return m_activePath; }
    QVariantList tracks() const { return value("track-list").toList(); }
    QVariantList chapters() const { return value("chapter-list").toList(); }
    QVariantList playlist() const { return value("playlist").toList(); }
    int playlistIndex() const { return value("playlist-pos",-1).toInt(); }
    int chapterIndex() const { return value("chapter",-1).toInt(); }
    bool looping() const { return value("loop-file","no").toString()!="no"; }
    QString abStart() const { return value("ab-loop-a","no").toString(); }
    QString abEnd() const { return value("ab-loop-b","no").toString(); }
    QString aspect() const;
    QString scaleMode() const;
    int rotation() const { return value("video-rotate",0).toInt(); }
    QString error() const { return m_error; }
    QVariantMap diagnostics() const { return m_diagnostics; }
    HistoryStore *history() { return &m_history; }
    void setResumeEnabled(bool value) { m_resumeEnabled=value; }
    void setAutoplay(bool value) { m_autoplay=value; }
    Q_INVOKABLE bool open(const QUrl &url, bool fromBeginning=false);
    Q_INVOKABLE bool openPath(const QString &path, bool fromBeginning=false);
    Q_INVOKABLE bool openFiles(const QList<QUrl> &urls, bool append=false);
    Q_INVOKABLE void playIndex(int index);
    Q_INVOKABLE void next();
    Q_INVOKABLE void previous();
    Q_INVOKABLE void removeQueueItem(int index);
    Q_INVOKABLE void clearQueue();
    Q_INVOKABLE void togglePause();
    Q_INVOKABLE void setPaused(bool pause);
    Q_INVOKABLE void stop();
    Q_INVOKABLE void seek(double seconds);
    Q_INVOKABLE void seekRelative(double seconds);
    Q_INVOKABLE void setVolume(double volume);
    Q_INVOKABLE void toggleMute();
    Q_INVOKABLE void setHardwareDecoding(bool enabled);
    Q_INVOKABLE void setSpeed(double value);
    Q_INVOKABLE void selectAudio(int id);
    Q_INVOKABLE void selectSubtitle(int id);
    Q_INVOKABLE bool loadSubtitle(const QUrl &url);
    Q_INVOKABLE void toggleSubtitles();
    Q_INVOKABLE void setAudioDelay(double seconds);
    Q_INVOKABLE void setSubtitleDelay(double seconds);
    Q_INVOKABLE void setSubtitleScale(double scale);
    Q_INVOKABLE void setSubtitlePosition(double position);
    Q_INVOKABLE void setAspect(const QString &aspect);
    Q_INVOKABLE void setScaleMode(const QString &mode);
    Q_INVOKABLE void setRotation(int degrees);
    Q_INVOKABLE void setLooping(bool enabled);
    Q_INVOKABLE void setAbStart();
    Q_INVOKABLE void setAbEnd();
    Q_INVOKABLE void clearAb();
    Q_INVOKABLE void frameStep();
    Q_INVOKABLE void setChapter(int index);
    Q_INVOKABLE void screenshot();
    Q_INVOKABLE void dismissError();
signals:
    void readyChanged();
    void stateChanged();
    void controlsChanged();
    void tracksChanged();
    void chaptersChanged();
    void queueChanged();
    void diagnosticsChanged();
    void errorChanged();
    void fileLoaded();
    void playbackEnded();
    void notification(QString message);
private:
    QVariant value(const QString &name,const QVariant &fallback={}) const { return m_values.value(name,fallback); }
    static void wakeup(void *context);
    void scheduleDrain();
    void drainEvents();
    void fail(const QString &message);
    void command(const QList<QByteArray> &arguments, uint64_t request=0);
    void setFlag(const char *name,bool value);
    void setNumber(const char *name,double value);
    QString checkedPath(const QUrl &url, bool subtitle=false);
    void saveResume();
    void applyResume();
    std::unique_ptr<MpvApi> m_api;
    mpv_handle *m_handle=nullptr;
    std::atomic_bool m_drainPending{false};
    bool m_loaded=false, m_resumeEnabled=true, m_autoplay=true, m_resumeApplied=false, m_pauseAtNextFile=false;
    int m_lastSaveBucket=-1;
    QString m_error,m_activePath,m_expectedPath,m_screenshotPath;
    QVariantMap m_values,m_diagnostics;
    QSet<QString> m_fromBeginning;
    HistoryStore m_history;
};
