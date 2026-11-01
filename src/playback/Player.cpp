#include "Player.h"
#include "MpvApi.h"
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <QStandardPaths>
#include <algorithm>
#include <cmath>
#include <vector>

Player::Player(QObject *parent) : QObject(parent),m_api(std::make_unique<MpvApi>()) {
    connect(&m_history,&HistoryStore::changed,this,[this] { if (m_loaded && !m_resumeApplied) applyResume(); });
}
Player::~Player() { shutdown(); }
void Player::fail(const QString &message) { m_error=message; emit errorChanged(); }
void Player::dismissError() { m_error.clear(); emit errorChanged(); }

bool Player::initialize(const QString &libraryPath,quintptr windowId,Output output) {
    if (m_handle) return true;
    QString error;
    if (!m_api->load(libraryPath,error)) { fail(error); return false; }
    m_handle=m_api->create();
    if (!m_handle) { fail("mpv_create failed."); return false; }
    auto option=[this](const char *key,const char *text) {
        const int result=m_api->set_option_string(m_handle,key,text);
        if (result<0) fail(QString::fromLatin1(key)+": "+m_api->error_string(result));
        return result>=0;
    };
    const std::pair<const char*,const char*> common[]={
        {"config","no"},{"load-scripts","no"},{"ytdl","no"},{"osc","no"},{"terminal","no"},
        {"input-default-bindings","no"},{"input-vo-keyboard","no"},{"input-cursor","no"},
        {"idle","yes"},{"keep-open","yes"},{"pause","yes"},{"audio-display","no"},
        {"interpolation","no"},{"video-sync","audio"},{"access-references","no"},
        {"load-unsafe-playlists","no"},{"demuxer-lavf-o","protocol_whitelist=[file]"},
        {"sub-auto","fuzzy"},{"osd-level","0"}
    };
    for (const auto &[key,text]:common) if (!option(key,text)) { shutdown(); return false; }
    if (output==Output::WindowsD3D11) {
        if (!windowId) { fail("A live native video window is required."); shutdown(); return false; }
        int64_t wid=static_cast<int64_t>(windowId);
        if (m_api->set_option(m_handle,"wid",MPV_FORMAT_INT64,&wid)<0) { fail("Cannot assign video HWND."); shutdown(); return false; }
        const std::pair<const char*,const char*> video[]={
            {"vo","gpu"},{"gpu-api","d3d11"},{"gpu-context","d3d11"},{"hwdec","d3d11va"},
            {"target-prim","bt.709"},{"target-trc","bt.1886"},{"d3d11-output-csp","srgb"},{"d3d11-output-format","rgba8"}
        };
        for (const auto &[key,text]:video) if (!option(key,text)) { shutdown(); return false; }
    } else {
        if (!option("vo","null") || !option("ao","null") || !option("hwdec","no")) { shutdown(); return false; }
    }
    const int result=m_api->initialize(m_handle);
    if (result<0) { fail(QStringLiteral("mpv_initialize: ")+m_api->error_string(result)); shutdown(); return false; }
    struct Observation { const char *name; mpv_format format; };
    const Observation observed[]={
        {"pause",MPV_FORMAT_FLAG},{"mute",MPV_FORMAT_FLAG},{"sub-visibility",MPV_FORMAT_FLAG},
        {"time-pos",MPV_FORMAT_DOUBLE},{"duration",MPV_FORMAT_DOUBLE},{"volume",MPV_FORMAT_DOUBLE},
        {"speed",MPV_FORMAT_DOUBLE},{"audio-delay",MPV_FORMAT_DOUBLE},{"sub-delay",MPV_FORMAT_DOUBLE},
        {"sub-scale",MPV_FORMAT_DOUBLE},{"sub-pos",MPV_FORMAT_DOUBLE},{"panscan",MPV_FORMAT_DOUBLE},
        {"video-zoom",MPV_FORMAT_DOUBLE},{"video-aspect-override",MPV_FORMAT_DOUBLE},
        {"video-rotate",MPV_FORMAT_STRING},{"playlist-pos",MPV_FORMAT_INT64},{"chapter",MPV_FORMAT_INT64},
        {"path",MPV_FORMAT_STRING},{"media-title",MPV_FORMAT_STRING},{"loop-file",MPV_FORMAT_STRING},
        {"ab-loop-a",MPV_FORMAT_STRING},{"ab-loop-b",MPV_FORMAT_STRING},
        {"track-list",MPV_FORMAT_NODE},{"chapter-list",MPV_FORMAT_NODE},{"playlist",MPV_FORMAT_NODE},
        {"mpv-version",MPV_FORMAT_STRING},{"ffmpeg-version",MPV_FORMAT_STRING},{"hwdec-current",MPV_FORMAT_STRING},
        {"video-codec",MPV_FORMAT_STRING},{"video-format",MPV_FORMAT_STRING},{"video-dec-params",MPV_FORMAT_NODE},
        {"video-params",MPV_FORMAT_NODE},{"audio-params",MPV_FORMAT_NODE},{"audio-codec-name",MPV_FORMAT_STRING},
        {"current-vo",MPV_FORMAT_STRING},{"current-ao",MPV_FORMAT_STRING},{"display-fps",MPV_FORMAT_DOUBLE},
        {"decoder-frame-drop-count",MPV_FORMAT_INT64},{"frame-drop-count",MPV_FORMAT_INT64}
    };
    for (const auto &p:observed) {
        if (m_api->observe_property(m_handle,0,p.name,p.format)<0) {
            fail(QStringLiteral("Property observation failed: ")+p.name); shutdown(); return false;
        }
    }
    m_api->set_wakeup_callback(m_handle,&Player::wakeup,this);
    dismissError(); emit readyChanged(); scheduleDrain(); return true;
}
void Player::shutdown() {
    if (!m_handle) return;
    saveResume();
    m_api->set_wakeup_callback(m_handle,nullptr,nullptr);
    m_api->terminate_destroy(m_handle); m_handle=nullptr;
    m_loaded=false; m_values.clear(); m_diagnostics.clear(); m_activePath.clear();
    emit readyChanged(); emit stateChanged(); emit diagnosticsChanged();
    emit controlsChanged();emit tracksChanged();emit chaptersChanged();emit queueChanged();
}
void Player::wakeup(void *context) { static_cast<Player*>(context)->scheduleDrain(); }
void Player::scheduleDrain() {
    if (m_drainPending.exchange(true)) return;
    QMetaObject::invokeMethod(this,[this] { drainEvents(); },Qt::QueuedConnection);
}
static QVariant nodeValue(const mpv_node &node) {
    switch (node.format) {
    case MPV_FORMAT_STRING:return QString::fromUtf8(node.u.string);
    case MPV_FORMAT_FLAG:return node.u.flag!=0;
    case MPV_FORMAT_INT64:return QVariant::fromValue<qlonglong>(node.u.int64);
    case MPV_FORMAT_DOUBLE:return node.u.double_;
    case MPV_FORMAT_NODE_ARRAY: {
        QVariantList list; for (int i=0;i<node.u.list->num;++i) list.append(nodeValue(node.u.list->values[i])); return list;
    }
    case MPV_FORMAT_NODE_MAP: {
        QVariantMap map; for (int i=0;i<node.u.list->num;++i) map.insert(QString::fromUtf8(node.u.list->keys[i]),nodeValue(node.u.list->values[i])); return map;
    }
    default:return {};
    }
}
void Player::drainEvents() {
    m_drainPending.store(false);
    if (!m_handle) return;
    bool stateDirty=false,diagnosticDirty=false,controlDirty=false,trackDirty=false,chapterDirty=false,queueDirty=false;
    const bool wasPaused=paused();
    for (int count=0;count<256;++count) {
        auto *event=m_api->wait_event(m_handle,0);
        if (event->event_id==MPV_EVENT_NONE) break;
        if (event->event_id==MPV_EVENT_PROPERTY_CHANGE) {
            const auto &p=*static_cast<mpv_event_property*>(event->data);
            const QString name=QString::fromUtf8(p.name);
            QVariant v;
            if (p.data) switch(p.format) {
            case MPV_FORMAT_NODE:v=nodeValue(*static_cast<mpv_node*>(p.data)); break;
            case MPV_FORMAT_STRING:v=QString::fromUtf8(*static_cast<char**>(p.data)); break;
            case MPV_FORMAT_FLAG:v=*static_cast<int*>(p.data)!=0; break;
            case MPV_FORMAT_DOUBLE:v=*static_cast<double*>(p.data); break;
            case MPV_FORMAT_INT64:v=QVariant::fromValue<qlonglong>(*static_cast<int64_t*>(p.data)); break;
            default:break;
            }
            bool changed=false;
            if(v.isValid()) { if(m_values.value(name)!=v) { m_values.insert(name,v);changed=true; } }
            else changed=m_values.remove(name)>0;
            if(changed) {
                if(name=="track-list")trackDirty=true;
                else if(name=="playlist" || name=="playlist-pos")queueDirty=true;
                else if(name=="chapter-list" || name=="chapter")chapterDirty=true;
                else if(QSet<QString>{"pause","mute","time-pos","duration","volume","media-title","path"}.contains(name))stateDirty=true;
                else controlDirty=true;
            }
            static const QSet<QString> diagnosticNames={"mpv-version","ffmpeg-version","hwdec-current","video-codec","video-format","video-dec-params","video-params","audio-params","audio-codec-name","current-vo","current-ao","display-fps","decoder-frame-drop-count","frame-drop-count"};
            if (diagnosticNames.contains(name)) {
                if (v.isValid()) { if (m_diagnostics.value(name)!=v) { m_diagnostics.insert(name,v); diagnosticDirty=true; } }
                else if (m_diagnostics.remove(name)) diagnosticDirty=true;
            }
            if (name=="path" && m_loaded && !v.toString().isEmpty() && m_activePath!=v.toString()) {
                m_activePath=v.toString(); applyResume();
            }
        } else if (event->event_id==MPV_EVENT_FILE_LOADED) {
            m_loaded=true;
            // One authoritative path query per file load, never progress polling.
            char *path=m_api->get_property_string(m_handle,"path");
            m_activePath=path?QString::fromUtf8(path):m_expectedPath;
            if(path)m_api->free(path);
            if(m_pauseAtNextFile){setPaused(true);m_pauseAtNextFile=false;}
            m_lastSaveBucket=-1; applyResume(); stateDirty=true; emit fileLoaded();
        } else if (event->event_id==MPV_EVENT_START_FILE) {
            saveResume(); m_loaded=false; m_resumeApplied=false; m_activePath.clear();
            m_values.remove("time-pos");m_values.remove("duration");m_values.remove("media-title");
            stateDirty=true;
        } else if (event->event_id==MPV_EVENT_END_FILE) {
            saveResume();
            auto *end=static_cast<mpv_event_end_file*>(event->data);
            m_pauseAtNextFile=end->reason==MPV_END_FILE_REASON_EOF && !m_autoplay;
            m_loaded=false; m_activePath.clear();
            m_values.remove("time-pos");m_values.remove("duration");m_values.remove("media-title"); stateDirty=true;
            if (end->reason==MPV_END_FILE_REASON_ERROR) fail(QStringLiteral("Playback failed: ")+m_api->error_string(end->error));
            emit playbackEnded();
        } else if (event->event_id==MPV_EVENT_COMMAND_REPLY || event->event_id==MPV_EVENT_SET_PROPERTY_REPLY) {
            if (event->error<0) fail(QStringLiteral("Playback operation failed: ")+m_api->error_string(event->error));
            else if (event->reply_userdata==1001) emit notification("Screenshot saved to "+m_screenshotPath);
        } else if (event->event_id==MPV_EVENT_QUEUE_OVERFLOW) fail("libmpv event queue overflow; restart playback.");
        if (count==255) scheduleDrain();
    }
    if(m_loaded && !m_resumeApplied)applyResume();
    if (m_loaded && ((int(position())/10)!=m_lastSaveBucket || (!wasPaused && paused()))) saveResume();
    if (stateDirty) emit stateChanged();
    if (diagnosticDirty) emit diagnosticsChanged();
    if(controlDirty)emit controlsChanged();
    if(trackDirty)emit tracksChanged();
    if(chapterDirty)emit chaptersChanged();
    if(queueDirty)emit queueChanged();
}
QString Player::checkedPath(const QUrl &url,bool subtitle) {
    const auto supplied=url.toLocalFile();
    if (!url.isLocalFile() || !url.host().isEmpty() || supplied.startsWith("//") || supplied.startsWith("\\\\")) {
        fail("Choose a readable local file. URLs and network shares are not accepted."); return {};
    }
    const QFileInfo file(supplied);
    const QSet<QString> videoExtensions={"mkv","mp4","mov","avi","webm","ts","mts","m2ts","m4v","flv","mpg","mpeg"};
    const QSet<QString> subtitleExtensions={"srt","ass","ssa","vtt","sub","sup"};
    const auto path=file.canonicalFilePath();
    if (!file.isFile() || !file.isReadable() || path.startsWith("//") || path.startsWith("\\\\") ||
        !(subtitle?subtitleExtensions:videoExtensions).contains(file.suffix().toLower())) {
        fail(subtitle?"Choose a readable local subtitle file.":"Choose a readable local video file. Playlist files are not accepted."); return {};
    }
    return path;
}
bool Player::open(const QUrl &url,bool fromBeginning) {
    if (!ready()) { fail("Playback engine is not ready."); return false; }
    const auto file=checkedPath(url); if(file.isEmpty()) return false;
    saveResume(); dismissError();m_expectedPath=file;m_pauseAtNextFile=false;
    if(fromBeginning) m_fromBeginning.insert(file);
    command({"loadfile",file.toUtf8(),"replace"}); setPaused(false); return true;
}
bool Player::openPath(const QString &path,bool fromBeginning){return open(QUrl::fromLocalFile(path),fromBeginning);}
bool Player::openFiles(const QList<QUrl> &urls,bool append) {
    if (!ready() || urls.isEmpty()) return false;
    QList<QString> files;
    for (const auto &url:urls) { const auto file=checkedPath(url);if(file.isEmpty()) return false;files.append(file); }
    dismissError();
    bool first=true;
    for(const auto &file:files) {
        if(first && !append) { saveResume();m_expectedPath=file;m_pauseAtNextFile=false;command({"loadfile",file.toUtf8(),"replace"});setPaused(false); }
        else command({"loadfile",file.toUtf8(),"append-play"});
        first=false;
    }
    return true;
}
void Player::command(const QList<QByteArray> &arguments,uint64_t request) {
    if(!m_handle) return;
    std::vector<const char*> pointers;for(const auto &arg:arguments)pointers.push_back(arg.constData());pointers.push_back(nullptr);
    const int result=m_api->command_async(m_handle,request,pointers.data());
    if(result<0)fail(QString::fromLatin1(m_api->error_string(result)));
}
void Player::setFlag(const char *name,bool flag) {
    if(!m_handle)return;
    int n=flag?1:0;
    const int r=m_api->set_property_async(m_handle,0,name,MPV_FORMAT_FLAG,&n);if(r<0)fail(QString::fromLatin1(m_api->error_string(r)));
}
void Player::setNumber(const char *name,double n) {
    if(!m_handle || !std::isfinite(n))return;
    const int r=m_api->set_property_async(m_handle,0,name,MPV_FORMAT_DOUBLE,&n);if(r<0)fail(QString::fromLatin1(m_api->error_string(r)));
}
void Player::togglePause(){if(m_loaded)command({"cycle","pause"});}
void Player::setPaused(bool v){setFlag("pause",v);}
void Player::stop(){saveResume();command({"stop"});}
void Player::seek(double n){if(m_loaded && std::isfinite(n))command({"seek",QByteArray::number(std::clamp(n,0.0,duration())),"absolute+exact"});}
void Player::seekRelative(double n){if(m_loaded && std::isfinite(n))command({"seek",QByteArray::number(n),"relative+exact"});}
void Player::setVolume(double n){if(std::isfinite(n))setNumber("volume",std::clamp(n,0.0,100.0));}
void Player::toggleMute(){command({"cycle","mute"});}
void Player::setHardwareDecoding(bool v){command({"set","hwdec",v?"d3d11va":"no"});}
void Player::setSpeed(double n){if(std::isfinite(n))setNumber("speed",std::clamp(n,0.5,2.0));}
void Player::setAudioDelay(double n){if(std::isfinite(n))setNumber("audio-delay",std::clamp(n,-10.0,10.0));}
void Player::setSubtitleDelay(double n){if(std::isfinite(n))setNumber("sub-delay",std::clamp(n,-60.0,60.0));}
void Player::setSubtitleScale(double n){if(std::isfinite(n))setNumber("sub-scale",std::clamp(n,0.5,3.0));}
void Player::setSubtitlePosition(double n){if(std::isfinite(n))setNumber("sub-pos",std::clamp(n,0.0,150.0));}
void Player::selectAudio(int id){for(const auto &t:tracks())if(t.toMap().value("type")=="audio" && t.toMap().value("id").toInt()==id){command({"set","aid",QByteArray::number(id)});return;}}
void Player::selectSubtitle(int id){if(id<0){command({"set","sid","no"});return;}for(const auto &t:tracks())if(t.toMap().value("type")=="sub" && t.toMap().value("id").toInt()==id){command({"set","sid",QByteArray::number(id)});setFlag("sub-visibility",true);return;}}
bool Player::loadSubtitle(const QUrl &url){const auto p=checkedPath(url,true);if(p.isEmpty() || !m_loaded)return false;command({"sub-add",p.toUtf8(),"select"});return true;}
void Player::toggleSubtitles(){command({"cycle","sub-visibility"});}
void Player::setAspect(const QString &a){const QSet<QString> valid={"Auto","16:9","16:10","4:3","21:9"};if(valid.contains(a))command({"set","video-aspect-override",a=="Auto"?QByteArray("-1"):a.toUtf8()});}
QString Player::aspect()const{const double n=value("video-aspect-override",-1).toDouble();for(const auto &p:QList<QPair<QString,double>>{{"16:9",16.0/9},{"16:10",1.6},{"4:3",4.0/3},{"21:9",21.0/9}})if(std::abs(n-p.second)<0.01)return p.first;return "Auto";}
void Player::setScaleMode(const QString &mode){if(mode!="Fit" && mode!="Fill" && mode!="Zoom")return;setNumber("panscan",mode=="Fill"?1:0);setNumber("video-zoom",mode=="Zoom"?0.25:0);}
QString Player::scaleMode()const{if(value("video-zoom",0).toDouble()>0)return "Zoom";return value("panscan",0).toDouble()>0?"Fill":"Fit";}
void Player::setRotation(int n){if(n==0 || n==90 || n==180 || n==270)command({"set","video-rotate",QByteArray::number(n)});}
void Player::setLooping(bool v){command({"set","loop-file",v?"inf":"no"});}
void Player::setAbStart(){if(m_loaded){command({"set","ab-loop-a",QByteArray::number(position())});command({"set","ab-loop-b","no"});}}
void Player::setAbEnd(){bool ok=false;const double a=abStart().toDouble(&ok);if(m_loaded && ok && position()>a)command({"set","ab-loop-b",QByteArray::number(position())});else fail("Set point A first, then seek or play beyond it to set point B.");}
void Player::clearAb(){command({"set","ab-loop-a","no"});command({"set","ab-loop-b","no"});}
void Player::frameStep(){if(m_loaded)command({"frame-step"});}
void Player::setChapter(int n){if(n>=0 && n<chapters().size())command({"set","chapter",QByteArray::number(n)});}
void Player::playIndex(int n){if(n>=0 && n<playlist().size()){saveResume();m_pauseAtNextFile=false;command({"set","playlist-pos",QByteArray::number(n)});setPaused(false);}}
void Player::next(){if(playlistIndex()+1<playlist().size()){saveResume();command({"playlist-next","force"});}}
void Player::previous(){if(playlistIndex()>0){saveResume();command({"playlist-prev","force"});}}
void Player::removeQueueItem(int n){if(n>=0 && n<playlist().size())command({"playlist-remove",QByteArray::number(n)});}
void Player::clearQueue(){command({"playlist-clear"});}
void Player::screenshot(){
    if(!m_loaded)return;
    const auto directory=QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)+"/Jadzumi Player";
    if(!QDir().mkpath(directory)){fail("Cannot create the screenshot folder.");return;}
    m_screenshotPath=directory+"/"+QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss-zzz")+".png";
    command({"screenshot-to-file",m_screenshotPath.toUtf8(),"subtitles"},1001);
}
void Player::applyResume(){
    if(!m_loaded || m_resumeApplied || !m_history.ready() || m_activePath.isEmpty() || duration()<=0)return;
    m_resumeApplied=true;
    const QFileInfo f(m_activePath);
    const bool fromStart=m_fromBeginning.remove(m_activePath)>0;
    if(m_resumeEnabled && !fromStart){
        const double p=m_history.resumePosition(m_activePath,f.size(),f.lastModified().toMSecsSinceEpoch());
        if(p>=2 && p<duration()-3){seek(p);emit notification("Resumed from your last position.");}
    }
}
void Player::saveResume(){
    if(!m_loaded || m_activePath.isEmpty() || duration()<=0 || !m_resumeApplied)return;
    const QFileInfo f(m_activePath);
    double p=position();if(p>=duration()-2)p=0;
    m_history.save({{"path",m_activePath},{"title",title().isEmpty()?f.fileName():title()},{"position",p},{"duration",duration()},
        {"size",f.size()},{"modified",f.lastModified().toMSecsSinceEpoch()},{"played",QDateTime::currentMSecsSinceEpoch()}});
    m_lastSaveBucket=int(position())/10;
}
