#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QQuickStyle>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QDateTime>
#include <QMutex>
#include <QMutexLocker>
#include <QTimer>
#include <QUrl>
#include <QIcon>
#include <QScreen>
#include <QCommandLineParser>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QAbstractNativeEventFilter>
#include <windows.h>
#include <dwmapi.h>
#include "playback/Player.h"
#include "rendering/VideoWindow.h"
#include "settings/Preferences.h"

static QMutex logMutex;
static QStringList qmlIssues;
static void localLog(QtMsgType type,const QMessageLogContext&,const QString &message) {
    const QMutexLocker lock(&logMutex);
    if(message.contains("ReferenceError") || message.contains("TypeError") || message.contains("Cannot assign") || message.contains("is not defined")) qmlIssues.append(message);
    const auto directory=QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)+"/logs";
    QDir().mkpath(directory);
    const auto path=directory+"/player.log";
    if(QFileInfo(path).size()>1024*1024){QFile::remove(path+".1");QFile::rename(path,path+".1");}
    QFile f(path);
    if(f.open(QIODevice::WriteOnly|QIODevice::Append))f.write((QDateTime::currentDateTimeUtc().toString(Qt::ISODate)+" ["+QString::number(type)+"] "+message+'\n').toUtf8());
}
class SystemSettingsFilter final:public QAbstractNativeEventFilter {
public:
    Preferences *preferences=nullptr;
    bool nativeEventFilter(const QByteArray&,void *message,qintptr*)override {
        if(static_cast<MSG*>(message)->message==WM_SETTINGCHANGE && preferences)
            QMetaObject::invokeMethod(preferences,[this]{preferences->refreshSystemMotion();},Qt::QueuedConnection);
        return false;
    }
};
int main(int argc,char **argv) {
    SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_APPLICATION_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);
    QQuickWindow::setGraphicsApi(QSGRendererInterface::Direct3D11);
    QQuickWindow::setDefaultAlphaBuffer(true);
    QGuiApplication app(argc,argv);
    app.setOrganizationName("Jadzumi");app.setApplicationName("Jadzumi Player Windows");app.setApplicationVersion("0.3.0-m2");
    app.setWindowIcon(QIcon(":/qt/qml/Jadzumi/assets/jadzumi.ico"));
    qInstallMessageHandler(localLog);
    QCommandLineParser parser;parser.setApplicationDescription("Jadzumi Player — local Windows video playback");parser.addHelpOption();parser.addVersionOption();
    parser.addPositionalArgument("videos","Local video files to open.","[videos...]");
    parser.addOption({"smoke-test","Run automated UI/playback checks with the supplied local media file.","file"});
    parser.addOption({"smoke-dir","Write smoke-test screenshots and JSON evidence to this directory.","directory","smoke-evidence"});
    parser.process(app);
    QQuickStyle::setStyle("Basic");
    QSettings settings;
    Preferences preferences;
    SystemSettingsFilter systemFilter;systemFilter.preferences=&preferences;app.installNativeEventFilter(&systemFilter);
    VideoWindow video;
    Player player;
    auto applyPreferences=[&]{player.setResumeEnabled(preferences.resumeEnabled());player.setAutoplay(preferences.autoplay());};
    applyPreferences();QObject::connect(&preferences,&Preferences::changed,&player,applyPreferences);
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("Player",&player);
    engine.rootContext()->setContextProperty("Preferences",&preferences);
    engine.rootContext()->setContextProperty("VideoSurface",&video);
    engine.load(QUrl("qrc:/qt/qml/Jadzumi/qml/Main.qml"));
    if(engine.rootObjects().isEmpty())return 1;
    auto *window=qobject_cast<QQuickWindow*>(engine.rootObjects().first());if(!window)return 1;
    window->resize(qBound(780,settings.value("window/width",1200).toInt(),7680),qBound(520,settings.value("window/height",800).toInt(),4320));
    const QPoint lastPosition=settings.value("window/position").toPoint();
    if(settings.contains("window/position"))for(const auto *screen:QGuiApplication::screens())if(screen->availableGeometry().contains(lastPosition+QPoint(100,100))){window->setPosition(lastPosition);break;}
    auto titleTheme=[window]{BOOL dark=window->color().lightnessF()<0.5;DwmSetWindowAttribute(reinterpret_cast<HWND>(window->winId()),20,&dark,sizeof(dark));};
    titleTheme();QObject::connect(window,&QQuickWindow::colorChanged,window,titleTheme);
    QObject::connect(&video,&VideoWindow::toggleFullscreenRequested,window,[window]{QMetaObject::invokeMethod(window,"toggleFullscreen");});
    QObject::connect(&player,&Player::errorChanged,&app,[&]{if(!player.error().isEmpty())qWarning().noquote()<<player.error();});
    const bool smoke=parser.isSet("smoke-test");
    const QString smokeDir=QFileInfo(parser.value("smoke-dir")).absoluteFilePath();
    QTimer::singleShot(0,&app,[&]{
        const auto library=QCoreApplication::applicationDirPath()+"/libmpv-2.dll";
        if(player.initialize(library,video.winId(),Player::Output::WindowsD3D11)) {
            player.setVolume(settings.value("playback/volume",80).toDouble());
            if(!smoke){QList<QUrl> files;for(const auto &f:parser.positionalArguments())files.append(QUrl::fromLocalFile(QFileInfo(f).absoluteFilePath()));if(!files.isEmpty())player.openFiles(files);}
        }
    });
    if(smoke) {
        QDir().mkpath(smokeDir);preferences.setTheme("light");player.setResumeEnabled(false);
        auto capture=[window,smokeDir](const QString &name){
            // Explicit test-only screen capture includes mpv and the native HUD child windows.
            if(window->screen())window->screen()->grabWindow(0).save(smokeDir+"/"+name+".png");
        };
        QTimer::singleShot(1000,&app,[=]{capture("01-home-light");});
        QTimer::singleShot(1400,&app,[&]{player.open(QUrl::fromLocalFile(QFileInfo(parser.value("smoke-test")).absoluteFilePath()),true);});
        QTimer::singleShot(3200,&app,[=]{capture("02-native-video");QMetaObject::invokeMethod(window,"openSettings");});
        QTimer::singleShot(4100,&app,[=]{capture("03-settings-light");QMetaObject::invokeMethod(window,"closeSettings");});
        QTimer::singleShot(4400,&app,[&]{preferences.setTheme("dark");});
        QTimer::singleShot(5100,&app,[=]{capture("04-player-dark");QMetaObject::invokeMethod(window,"toggleFullscreen");});
        QTimer::singleShot(6000,&app,[=]{capture("05-fullscreen");QMetaObject::invokeMethod(window,"toggleFullscreen");});
        QTimer::singleShot(6500,&app,[&]{preferences.setTheme("amoled");});
        QTimer::singleShot(7300,&app,[=]{capture("06-player-amoled");});
        QTimer::singleShot(7800,&app,[&]{preferences.setTheme("system");preferences.setReducedMotion(true);});
        QTimer::singleShot(8500,&app,[=]{QMetaObject::invokeMethod(window,"openSettings");});
        QTimer::singleShot(9200,&app,[=]{capture("07-settings-system-reduced-motion");QMetaObject::invokeMethod(window,"closeSettings");});
        QTimer::singleShot(10500,&app,[&]{
            QStringList issues;{const QMutexLocker lock(&logMutex);issues=qmlIssues;}
            const bool ok=player.ready() && player.loaded() && player.position()>0 && player.error().isEmpty() && issues.isEmpty() && player.diagnostics().value("current-vo").toString()=="gpu";
            QJsonObject report{{"success",ok},{"version",app.applicationVersion()},{"position",player.position()},{"error",player.error()},
                {"qmlIssues",QJsonArray::fromStringList(issues)},{"diagnostics",QJsonObject::fromVariantMap(player.diagnostics())},
                {"limits","Virtual Windows runner: physical laptop decoding, audible sync and HDR output require separate verification."}};
            QFile f(smokeDir+"/smoke-report.json");if(f.open(QIODevice::WriteOnly))f.write(QJsonDocument(report).toJson());
            app.exit(ok?0:2);
        });
    }
    QObject::connect(&app,&QCoreApplication::aboutToQuit,&app,[&]{
        if(window->visibility()==QWindow::Windowed){settings.setValue("window/width",window->width());settings.setValue("window/height",window->height());settings.setValue("window/position",window->position());}
        settings.setValue("playback/volume",player.volume());settings.sync();player.shutdown();
    });
    const int result=app.exec();player.shutdown();app.removeNativeEventFilter(&systemFilter);return result;
}
