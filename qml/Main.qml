import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtQml.Models
import Jadzumi

ApplicationWindow {
    id: root
    visible: true; width: 1200; height: 800
    minimumWidth: 780; minimumHeight: 520
    title: (Player.title.length ? Player.title + " — " : "") + "Jadzumi Player"
    color: Theme.canvas
    font.family: Theme.fontFamily; font.pixelSize: 14 * Theme.scaleFactor
    palette.window: Theme.canvas; palette.windowText: Theme.text; palette.text: Theme.text
    palette.base: Theme.surface; palette.button: Theme.raised; palette.buttonText: Theme.text
    palette.highlight: Theme.accent; palette.highlightedText: Theme.onAccent
    property string page: "home"
    property bool appendFiles: false
    property string notice: ""
    readonly property bool shortcutsAllowed: !homeSettings.opened && !details.opened && !videoDialog.visible && !hud.modalOpen
    function localUrl(path) { const p = path.replace(/\\/g, "/"); return p.startsWith("/") ? "file://" + p : "file:///" + p }
    function toggleFullscreen() { if (visibility === Window.FullScreen) showNormal(); else showFullScreen(); hud.showControls() }
    function openSettings() { if (page === "player") hud.openSettings(); else homeSettings.open() }
    function closeSettings() { homeSettings.close(); hud.closeSettings() }
    function showHome() { if (Player.loaded) Player.setPaused(true); page = "home"; if (visibility === Window.FullScreen) showNormal() }
    function chooseFiles(append) { appendFiles = append; videoDialog.open() }
    function act(action) {
        hud.showControls()
        switch (action) {
        case "open": chooseFiles(false); break
        case "playPause": Player.togglePause(); break
        case "seekBack": Player.seekRelative(-10); break
        case "seekForward": Player.seekRelative(10); break
        case "volumeUp": Player.setVolume(Player.volume + 5); break
        case "volumeDown": Player.setVolume(Player.volume - 5); break
        case "fullscreen": toggleFullscreen(); break
        case "mute": Player.toggleMute(); break
        case "subtitles": Player.toggleSubtitles(); break
        case "frameStep": Player.frameStep(); break
        case "next": Player.next(); break
        case "previous": Player.previous(); break
        case "stop": Player.stop(); break
        }
    }
    onActiveChanged: if (active) Preferences.refreshSystemMotion()
    Connections {
        target: Player
        function onFileLoaded() { root.page = "player"; hud.showControls() }
        function onNotification(message) { root.notice = message; noticeTimer.restart() }
    }
    Timer { id: noticeTimer; interval: 4000; onTriggered: root.notice = "" }
    FileDialog {
        id: videoDialog; title: root.appendFiles ? "Add local videos to queue" : "Open local videos"
        fileMode: FileDialog.OpenFiles
        nameFilters: ["Video files (*.mkv *.mp4 *.mov *.avi *.webm *.ts *.mts *.m2ts *.m4v *.flv *.mpg *.mpeg)"]
        onAccepted: Player.openFiles(selectedFiles, root.appendFiles)
    }
    Instantiator {
        model: Object.keys(Preferences.shortcuts)
        delegate: Shortcut {
            required property string modelData
            sequence: Preferences.shortcuts[modelData]; context: Qt.ApplicationShortcut
            enabled: root.shortcutsAllowed
            onActivated: root.act(modelData)
        }
    }
    Shortcut { sequence: "F11"; context: Qt.ApplicationShortcut; enabled: root.shortcutsAllowed; onActivated: root.toggleFullscreen() }
    Shortcut { sequence: "Esc"; context: Qt.ApplicationShortcut; enabled: root.shortcutsAllowed; onActivated: { if (root.visibility === Window.FullScreen) root.showNormal(); else hud.showControls() } }
    Shortcut { sequence: "Media Play"; context: Qt.ApplicationShortcut; enabled: root.shortcutsAllowed; onActivated: Player.setPaused(false) }
    Shortcut { sequence: "Media Pause"; context: Qt.ApplicationShortcut; enabled: root.shortcutsAllowed; onActivated: Player.setPaused(true) }
    DropArea { anchors.fill: parent; onDropped: drop => { if (drop.hasUrls) { Player.openFiles(drop.urls, false); drop.accepted = true } } }
    RowLayout {
        anchors.fill: parent; anchors.margins: root.page === "player" ? 0 : 20; spacing: 24
        Card {
            id: navigation
            property real navigationWidth: Preferences.expandedSidebar && root.width > 1000 ? 202 : 76
            visible: root.page !== "player"
            Layout.preferredWidth: navigationWidth; Layout.fillHeight: true
            Behavior on navigationWidth { NumberAnimation { duration: Theme.motion; easing.type: Easing.OutCubic } }
            ColumnLayout {
                anchors.fill: parent; anchors.margins: 14; spacing: 12
                RowLayout {
                    Layout.fillWidth: true
                    Rectangle { implicitWidth: 42; implicitHeight: 42; radius: 15; color: Theme.accent; Icon { name: "play"; tint: Theme.onAccent; anchors.centerIn: parent; width: 24; height: 24 } }
                    Label { textFormat: Text.PlainText; visible: navigation.navigationWidth > 100; text: "JADZUMI"; font.bold: true; font.pixelSize: 16; font.letterSpacing: 1.3 }
                }
                Item { implicitHeight: 18 }
                JButton { Layout.fillWidth: true; text: navigation.navigationWidth > 100 ? "Home" : ""; iconName: "home"; hint: "Home"; tone: root.page === "home" ? "primary" : "quiet"; onClicked: root.page = "home" }
                JButton { Layout.fillWidth: true; text: navigation.navigationWidth > 100 ? "Queue" : ""; iconName: "queue"; hint: "Queue"; tone: root.page === "queue" ? "primary" : "quiet"; onClicked: root.page = "queue" }
                JButton { Layout.fillWidth: true; text: navigation.navigationWidth > 100 ? "Open video" : ""; iconName: "open"; hint: "Open local videos"; tone: "quiet"; enabled: Player.ready; onClicked: root.chooseFiles(false) }
                JButton { visible: Player.loaded; Layout.fillWidth: true; text: navigation.navigationWidth > 100 ? "Now playing" : ""; iconName: "screen"; hint: "Now playing"; tone: "quiet"; onClicked: root.page = "player" }
                Item { Layout.fillHeight: true }
                JButton { Layout.fillWidth: true; text: navigation.navigationWidth > 100 ? "Settings" : ""; iconName: "settings"; hint: "Settings"; tone: "quiet"; onClicked: root.openSettings() }
                JButton { Layout.fillWidth: true; text: navigation.navigationWidth > 100 ? "Collapse" : ""; iconName: "menu"; hint: "Toggle sidebar"; tone: "quiet"; onClicked: Preferences.expandedSidebar = !Preferences.expandedSidebar }
            }
        }
        Item {
            Layout.fillWidth: true; Layout.fillHeight: true
            ColumnLayout {
                anchors.fill: parent; anchors.topMargin: 12; spacing: 16
                visible: root.page !== "player"
                Card {
                    visible: Player.error.length > 0
                    Layout.fillWidth: true; implicitHeight: errorMessage.implicitHeight + 24
                    RowLayout { id: errorMessage; anchors.fill: parent; anchors.margins: 12; Label { textFormat: Text.PlainText; text: Player.error; wrapMode: Text.Wrap; color: Theme.danger; Layout.fillWidth: true } JButton { compact: true; tone: "quiet"; iconName: "close"; hint: "Dismiss error"; onClicked: Player.dismissError() } }
                }
                HomeView { visible: root.page === "home"; Layout.fillWidth: true; Layout.fillHeight: true; onOpenRequested: append => root.chooseFiles(append); onDetailsRequested: record => { details.record = record; details.open() } }
                QueueView { visible: root.page === "queue"; Layout.fillWidth: true; Layout.fillHeight: true; onAddRequested: root.chooseFiles(true) }
                Label { textFormat: Text.PlainText; visible: root.notice.length > 0; text: root.notice; color: Theme.accent; elide: Text.ElideRight; Layout.fillWidth: true }
            }
            Item {
                anchors.fill: parent; visible: root.page === "player"
                Rectangle { anchors.fill: parent; color: "black" }
                WindowContainer { anchors.fill: parent; window: VideoSurface }
                // A separate transparent native child window is required above mpv's HWND.
                WindowContainer {
                    anchors.fill: parent
                    window: ApplicationWindow {
                        id: videoOverlay
                        color: "transparent"; flags: Qt.FramelessWindowHint
                        font.family: Theme.fontFamily; font.pixelSize: 14 * Theme.scaleFactor
                        palette.window: Theme.surface; palette.windowText: Theme.text; palette.text: Theme.text
                        palette.button: Theme.raised; palette.buttonText: Theme.text; palette.base: Theme.surface
                        palette.highlight: Theme.accent; palette.highlightedText: Theme.onAccent
                        PlayerHud { id: hud; anchors.fill: parent; onBackRequested: root.showHome(); onFullscreenRequested: root.toggleFullscreen(); onOpenRequested: append => root.chooseFiles(append) }
                    }
                }
            }
        }
    }
    SettingsPanel { id: homeSettings }
    FileDetails { id: details; onPlayRequested: (path, fromBeginning) => Player.openPath(path, fromBeginning) }
}
