#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QQuickStyle>
#include <QTimer>
#include <QDir>
#include <QDebug>
#include "playback/Player.h"
#include "settings/Preferences.h"
#include "settings/ThemeController.h"

// The same QML and real playback backend under a software UI test renderer.
// This harness is never packaged as the Windows application.
int main(int argc,char **argv){
    QQuickWindow::setSceneGraphBackend("software");
    QGuiApplication app(argc,argv);
    app.setOrganizationName("JadzumiTests");app.setApplicationName("UI smoke host");app.setApplicationVersion("0.3.0-ui-test");
    QQuickStyle::setStyle("Basic");
    Preferences preferences;preferences.setTheme("light");
    ThemeController theme(preferences);
    Player player;QWindow video;
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("Preferences",&preferences);
    engine.rootContext()->setContextProperty("ThemeState",&theme);
    engine.rootContext()->setContextProperty("Player",&player);
    engine.rootContext()->setContextProperty("VideoSurface",&video);
    qInfo("UI host: loading actual application QML");
    engine.load(app.arguments().size()>1?QUrl::fromLocalFile(app.arguments().at(1)):QUrl("qrc:/qt/qml/Jadzumi/qml/Main.qml"));
    if(engine.rootObjects().isEmpty())return 1;
    auto *window=qobject_cast<QQuickWindow*>(engine.rootObjects().first());
    if(!window)return 2;
    QDir().mkpath("build/ui-evidence");
    QTimer::singleShot(800,&app,[&]{window->grabWindow().save("build/ui-evidence/home-light.png");QMetaObject::invokeMethod(window,"openSettings");});
    QTimer::singleShot(1600,&app,[&]{window->grabWindow().save("build/ui-evidence/settings-light.png");QMetaObject::invokeMethod(window,"closeSettings");preferences.setTheme("dark");});
    QTimer::singleShot(2400,&app,[&]{window->grabWindow().save("build/ui-evidence/home-dark.png");app.quit();});
    return app.exec();
}
