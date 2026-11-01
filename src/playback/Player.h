#pragma once
#include <QObject>
#include <QUrl>
#include <QVariantMap>
#include <QList>
#include <QByteArray>
#include <atomic>
#include <memory>

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
    Q_PROPERTY(QString title READ title NOTIFY stateChanged)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)
    Q_PROPERTY(QVariantMap diagnostics READ diagnostics NOTIFY diagnosticsChanged)
public:
    enum class Output { WindowsD3D11, HeadlessTest };
    explicit Player(QObject *parent = nullptr);
    ~Player() override;
    bool initialize(const QString &libraryPath, quintptr windowId, Output output);
    void shutdown();
    bool ready() const { return m_handle != nullptr; }
    bool paused() const { return m_paused; }
    bool loaded() const { return m_loaded; }
    bool muted() const { return m_muted; }
    double position() const { return m_position; }
    double duration() const { return m_duration; }
    double volume() const { return m_volume; }
    QString title() const { return m_title; }
    QString error() const { return m_error; }
    QVariantMap diagnostics() const { return m_diagnostics; }
    Q_INVOKABLE bool open(const QUrl &url);
    Q_INVOKABLE void togglePause();
    Q_INVOKABLE void setPaused(bool pause);
    Q_INVOKABLE void stop();
    Q_INVOKABLE void seek(double seconds);
    Q_INVOKABLE void seekRelative(double seconds);
    Q_INVOKABLE void setVolume(double volume);
    Q_INVOKABLE void toggleMute();
    Q_INVOKABLE void setHardwareDecoding(bool enabled);
signals:
    void readyChanged();
    void stateChanged();
    void diagnosticsChanged();
    void errorChanged();
    void fileLoaded();
    void playbackEnded();
private:
    static void wakeup(void *context);
    void scheduleDrain();
    void drainEvents();
    void fail(const QString &message);
    void command(const QList<QByteArray> &arguments);
    void setFlag(const char *name, bool value);
    void setNumber(const char *name, double value);
    std::unique_ptr<MpvApi> m_api;
    mpv_handle *m_handle = nullptr;
    std::atomic_bool m_drainPending{false};
    bool m_paused = true, m_loaded = false, m_muted = false;
    double m_position = 0, m_duration = 0, m_volume = 100;
    QString m_title, m_error;
    QVariantMap m_diagnostics;
};
