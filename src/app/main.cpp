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
#include <windows.h>
#include "playback/Player.h"
#include "rendering/VideoWindow.h"

static void localLog(QtMsgType type, const QMessageLogContext &, const QString &message) {
    static QMutex mutex;
    const QMutexLocker lock(&mutex);
    const auto directory = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/logs";
    QDir().mkpath(directory);
    const auto path = directory + "/m0.log";
    if (QFileInfo(path).size() > 1024 * 1024) {
        QFile::remove(path + ".1"); QFile::rename(path, path + ".1");
    }
    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Append))
        file.write((QDateTime::currentDateTimeUtc().toString(Qt::ISODate) + " [" + QString::number(type) + "] " + message + '\n').toUtf8());
}

int main(int argc, char **argv) {
    // Restrict dependent DLL resolution. The selected mpv DLL is loaded by absolute path.
    SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_APPLICATION_DIR | LOAD_LIBRARY_SEARCH_SYSTEM32);
    QQuickWindow::setGraphicsApi(QSGRendererInterface::Direct3D11);
    QGuiApplication app(argc, argv);
    app.setOrganizationName("Jadzumi");
    app.setApplicationName("Jadzumi Player Windows");
    app.setApplicationVersion("0.1.0-m0");
    qInstallMessageHandler(localLog);
    QQuickStyle::setStyle("Basic");
    QSettings settings;
    VideoWindow video;
    Player player;
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("Player", &player);
    engine.rootContext()->setContextProperty("VideoSurface", &video);
    engine.load(QUrl(QStringLiteral("qrc:/qt/qml/Jadzumi/qml/Main.qml")));
    if (engine.rootObjects().isEmpty()) return 1;
    auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().first());
    if (!window) return 1;
    // Persist logical size; leave placement to Windows so unplugged monitors cannot hide the app.
    window->resize(qBound(720, settings.value("window/width", 1100).toInt(), 7680),
                   qBound(480, settings.value("window/height", 760).toInt(), 4320));
    QObject::connect(&video, &VideoWindow::toggleFullscreenRequested, window, [window] {
        if (window->visibility() == QWindow::FullScreen) window->showNormal(); else window->showFullScreen();
    });
    QObject::connect(&player, &Player::errorChanged, &app, [&player] {
        if (!player.error().isEmpty()) qWarning().noquote() << player.error();
    });
    QTimer::singleShot(0, &app, [&] {
        const auto library = QCoreApplication::applicationDirPath() + "/libmpv-2.dll";
        if (player.initialize(library, video.winId(), Player::Output::WindowsD3D11)) {
            player.setVolume(settings.value("playback/volume", 80).toDouble());
            if (app.arguments().size() > 1) player.open(QUrl::fromLocalFile(QFileInfo(app.arguments().at(1)).absoluteFilePath()));
        }
    });
    QObject::connect(&app, &QCoreApplication::aboutToQuit, &app, [&] {
        if (window->visibility() != QWindow::FullScreen && window->visibility() != QWindow::Minimized) {
            settings.setValue("window/width", window->width());
            settings.setValue("window/height", window->height());
        }
        settings.setValue("playback/volume", player.volume());
        settings.sync();
        player.shutdown(); // Before QML destroys its WindowContainer / video HWND.
    });
    const int result = app.exec();
    player.shutdown();
    return result;
}
