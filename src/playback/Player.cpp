#include "Player.h"
#include "MpvApi.h"
#include <QFileInfo>
#include <QSet>
#include <algorithm>
#include <cmath>
#include <vector>

Player::Player(QObject *parent) : QObject(parent), m_api(std::make_unique<MpvApi>()) {}
Player::~Player() { shutdown(); }

void Player::fail(const QString &message) {
    m_error = message;
    emit errorChanged();
}

bool Player::initialize(const QString &libraryPath, quintptr windowId, Output output) {
    if (m_handle) return true;
    QString error;
    if (!m_api->load(libraryPath, error)) { fail(error); return false; }
    m_handle = m_api->create();
    if (!m_handle) { fail(QStringLiteral("mpv_create failed.")); return false; }
    auto option = [this](const char *key, const char *value) {
        int result = m_api->set_option_string(m_handle, key, value);
        if (result < 0) fail(QString::fromLatin1(key) + ": " + m_api->error_string(result));
        return result >= 0;
    };
    // Refuse foreign config/scripts. Local file validation is also performed before loadfile.
    const std::pair<const char *, const char *> common[] = {
        {"config", "no"}, {"load-scripts", "no"}, {"ytdl", "no"},
        {"osc", "no"}, {"terminal", "no"}, {"input-default-bindings", "no"},
        {"input-vo-keyboard", "no"}, {"input-cursor", "no"},
        {"idle", "yes"}, {"keep-open", "yes"}, {"pause", "yes"},
        {"audio-display", "no"}, {"interpolation", "no"}, {"video-sync", "audio"},
        {"access-references", "no"}, {"load-unsafe-playlists", "no"},
        {"demuxer-lavf-o", "protocol_whitelist=[file]"}
    };
    for (const auto &[key, value] : common) {
        if (!option(key, value)) { shutdown(); return false; }
    }
    if (output == Output::WindowsD3D11) {
        if (!windowId) { fail(QStringLiteral("A live native video window is required.")); shutdown(); return false; }
        int64_t wid = static_cast<int64_t>(windowId);
        int result = m_api->set_option(m_handle, "wid", MPV_FORMAT_INT64, &wid);
        if (result < 0) { fail(QStringLiteral("Cannot assign video HWND.")); shutdown(); return false; }
        // SDR output until Windows display/HDR validation in M4. No claimed passthrough.
        const std::pair<const char *, const char *> video[] = {
            {"vo", "gpu"}, {"gpu-api", "d3d11"}, {"gpu-context", "d3d11"},
            {"hwdec", "d3d11va"}, {"target-prim", "bt.709"}, {"target-trc", "bt.1886"},
            {"d3d11-output-csp", "srgb"}, {"d3d11-output-format", "rgba8"}
        };
        for (const auto &[key, value] : video) {
            if (!option(key, value)) { shutdown(); return false; }
        }
    } else {
        if (!option("vo", "null") || !option("ao", "null") || !option("hwdec", "no")) {
            shutdown(); return false;
        }
    }
    int result = m_api->initialize(m_handle);
    if (result < 0) { fail(QStringLiteral("mpv_initialize: ") + m_api->error_string(result)); shutdown(); return false; }
    struct Observation { const char *name; mpv_format format; };
    const Observation observed[] = {
        {"pause", MPV_FORMAT_FLAG}, {"mute", MPV_FORMAT_FLAG},
        {"time-pos", MPV_FORMAT_DOUBLE}, {"duration", MPV_FORMAT_DOUBLE},
        {"volume", MPV_FORMAT_DOUBLE}, {"media-title", MPV_FORMAT_STRING},
        {"mpv-version", MPV_FORMAT_STRING}, {"ffmpeg-version", MPV_FORMAT_STRING},
        {"hwdec-current", MPV_FORMAT_STRING}, {"video-codec", MPV_FORMAT_STRING},
        {"video-format", MPV_FORMAT_STRING}, {"video-dec-params", MPV_FORMAT_NODE},
        {"video-params", MPV_FORMAT_NODE}, {"audio-params", MPV_FORMAT_NODE},
        {"audio-codec-name", MPV_FORMAT_STRING}, {"current-vo", MPV_FORMAT_STRING},
        {"current-ao", MPV_FORMAT_STRING}, {"display-fps", MPV_FORMAT_DOUBLE},
        {"decoder-frame-drop-count", MPV_FORMAT_INT64}, {"frame-drop-count", MPV_FORMAT_INT64}
    };
    for (const auto &property : observed) {
        result = m_api->observe_property(m_handle, 0, property.name, property.format);
        if (result < 0) { fail(QStringLiteral("Property observation failed: ") + property.name); shutdown(); return false; }
    }
    m_api->set_wakeup_callback(m_handle, &Player::wakeup, this);
    m_error.clear(); emit errorChanged(); emit readyChanged();
    scheduleDrain();
    return true;
}

void Player::shutdown() {
    if (!m_handle) return;
    // Keep this object and the video window alive until mpv's worker threads have stopped.
    m_api->set_wakeup_callback(m_handle, nullptr, nullptr);
    m_api->terminate_destroy(m_handle);
    m_handle = nullptr;
    m_loaded = false; m_paused = true; m_position = 0; m_duration = 0;
    m_title.clear(); m_diagnostics.clear();
    emit readyChanged(); emit stateChanged(); emit diagnosticsChanged();
}

void Player::wakeup(void *context) { static_cast<Player *>(context)->scheduleDrain(); }
void Player::scheduleDrain() {
    if (m_drainPending.exchange(true)) return;
    QMetaObject::invokeMethod(this, [this] { drainEvents(); }, Qt::QueuedConnection);
}

static QVariant nodeValue(const mpv_node &node) {
    switch (node.format) {
    case MPV_FORMAT_STRING: return QString::fromUtf8(node.u.string);
    case MPV_FORMAT_FLAG: return node.u.flag != 0;
    case MPV_FORMAT_INT64: return QVariant::fromValue<qlonglong>(node.u.int64);
    case MPV_FORMAT_DOUBLE: return node.u.double_;
    case MPV_FORMAT_NODE_MAP: {
        QVariantMap map;
        for (int i = 0; i < node.u.list->num; ++i)
            map.insert(QString::fromUtf8(node.u.list->keys[i]), nodeValue(node.u.list->values[i]));
        return map;
    }
    default: return {};
    }
}

void Player::drainEvents() {
    // Clear before draining: a racing callback always leaves another queued drain.
    m_drainPending.store(false);
    if (!m_handle) return;
    bool stateDirty = false, diagnosticDirty = false;
    for (int count = 0; count < 256; ++count) {
        auto *event = m_api->wait_event(m_handle, 0);
        if (event->event_id == MPV_EVENT_NONE) break;
        if (event->event_id == MPV_EVENT_PROPERTY_CHANGE) {
            const auto &p = *static_cast<mpv_event_property *>(event->data);
            const QByteArray name(p.name);
            if (p.format == MPV_FORMAT_NONE || !p.data) {
                if (name == "time-pos") { m_position = 0; stateDirty = true; }
                else if (name == "duration") { m_duration = 0; stateDirty = true; }
                else if (m_diagnostics.remove(QString::fromUtf8(name))) diagnosticDirty = true;
                continue;
            }
            if (name == "pause") { m_paused = *static_cast<int *>(p.data); stateDirty = true; }
            else if (name == "mute") { m_muted = *static_cast<int *>(p.data); stateDirty = true; }
            else if (name == "time-pos") { m_position = *static_cast<double *>(p.data); stateDirty = true; }
            else if (name == "duration") { m_duration = *static_cast<double *>(p.data); stateDirty = true; }
            else if (name == "volume") { m_volume = *static_cast<double *>(p.data); stateDirty = true; }
            else if (name == "media-title") { m_title = QString::fromUtf8(*static_cast<char **>(p.data)); stateDirty = true; }
            else {
                QVariant value;
                if (p.format == MPV_FORMAT_NODE) value = nodeValue(*static_cast<mpv_node *>(p.data));
                else if (p.format == MPV_FORMAT_STRING) value = QString::fromUtf8(*static_cast<char **>(p.data));
                else if (p.format == MPV_FORMAT_DOUBLE) value = *static_cast<double *>(p.data);
                else if (p.format == MPV_FORMAT_INT64) value = QVariant::fromValue<qlonglong>(*static_cast<int64_t *>(p.data));
                const auto key = QString::fromUtf8(name);
                if (m_diagnostics.value(key) != value) { m_diagnostics.insert(key, value); diagnosticDirty = true; }
            }
        } else if (event->event_id == MPV_EVENT_FILE_LOADED) {
            m_loaded = true; stateDirty = true; emit fileLoaded();
        } else if (event->event_id == MPV_EVENT_START_FILE) {
            m_loaded = false; m_position = 0; m_duration = 0; m_title.clear();
            // Never show the previous stream's diagnostics during the next file load.
            for (auto it = m_diagnostics.begin(); it != m_diagnostics.end();) {
                if (it.key() == "mpv-version" || it.key() == "ffmpeg-version") ++it;
                else it = m_diagnostics.erase(it);
            }
            stateDirty = true; diagnosticDirty = true;
        } else if (event->event_id == MPV_EVENT_END_FILE) {
            auto *end = static_cast<mpv_event_end_file *>(event->data);
            m_loaded = false; m_position = 0; m_duration = 0; m_title.clear();
            stateDirty = true;
            if (end->reason == MPV_END_FILE_REASON_ERROR)
                fail(QStringLiteral("Playback failed: ") + m_api->error_string(end->error));
            emit playbackEnded();
        } else if ((event->event_id == MPV_EVENT_COMMAND_REPLY || event->event_id == MPV_EVENT_SET_PROPERTY_REPLY)
                   && event->error < 0) {
            fail(QStringLiteral("Playback operation failed: ") + m_api->error_string(event->error));
        } else if (event->event_id == MPV_EVENT_QUEUE_OVERFLOW) {
            fail(QStringLiteral("libmpv event queue overflow; restart playback."));
        }
        if (count == 255) scheduleDrain();
    }
    if (stateDirty) emit stateChanged();
    if (diagnosticDirty) emit diagnosticsChanged();
}

bool Player::open(const QUrl &url) {
    if (!m_handle) { fail(QStringLiteral("Playback engine is not ready.")); return false; }
    const auto suppliedPath = url.toLocalFile();
    if (!url.isLocalFile() || !url.host().isEmpty() || suppliedPath.startsWith("//") || suppliedPath.startsWith("\\\\")) {
        fail(QStringLiteral("Choose a local video file. URLs and network shares are not accepted in M0."));
        return false;
    }
    const QFileInfo file(url.toLocalFile());
    static const QSet<QString> extensions = {"mkv", "mp4", "mov", "avi", "webm", "ts", "mts", "m2ts", "m4v", "flv", "mpg", "mpeg"};
    const QString path = file.canonicalFilePath();
    if (path.startsWith("//") || path.startsWith("\\\\") ||
        !file.isFile() || !file.isReadable() || !extensions.contains(file.suffix().toLower())) {
        fail(QStringLiteral("Choose a readable local video file. URLs, network shares and playlists are not accepted in M0."));
        return false;
    }
    m_error.clear(); emit errorChanged();
    command({"loadfile", path.toUtf8(), "replace"});
    setPaused(false);
    return true;
}

void Player::command(const QList<QByteArray> &arguments) {
    if (!m_handle) return;
    std::vector<const char *> pointers;
    for (const auto &arg : arguments) pointers.push_back(arg.constData());
    pointers.push_back(nullptr);
    const int result = m_api->command_async(m_handle, 0, pointers.data());
    if (result < 0) fail(QString::fromLatin1(m_api->error_string(result)));
}
void Player::setFlag(const char *name, bool value) {
    if (!m_handle) return;
    int flag = value ? 1 : 0;
    int result = m_api->set_property_async(m_handle, 0, name, MPV_FORMAT_FLAG, &flag);
    if (result < 0) fail(QString::fromLatin1(m_api->error_string(result)));
}
void Player::setNumber(const char *name, double value) {
    if (!m_handle || !std::isfinite(value)) return;
    int result = m_api->set_property_async(m_handle, 0, name, MPV_FORMAT_DOUBLE, &value);
    if (result < 0) fail(QString::fromLatin1(m_api->error_string(result)));
}
void Player::togglePause() { if (m_loaded) command({"cycle", "pause"}); }
void Player::setPaused(bool pause) { setFlag("pause", pause); }
void Player::stop() { command({"stop"}); }
void Player::seek(double seconds) {
    if (m_loaded && std::isfinite(seconds))
        command({"seek", QByteArray::number(std::clamp(seconds, 0.0, m_duration)), "absolute+exact"});
}
void Player::seekRelative(double seconds) {
    if (m_loaded && std::isfinite(seconds)) command({"seek", QByteArray::number(seconds), "relative+exact"});
}
void Player::setVolume(double volume) { setNumber("volume", std::clamp(volume, 0.0, 100.0)); }
void Player::toggleMute() { command({"cycle", "mute"}); }
void Player::setHardwareDecoding(bool enabled) {
    // Explicit user choice. A decoder that cannot initialize falls back inside mpv.
    command({"set", "hwdec", enabled ? "d3d11va" : "no"});
}
