import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import Jadzumi
Item {
    id: hud
    signal backRequested()
    signal fullscreenRequested()
    signal openRequested(bool append)
    property bool controlsShown: true
    readonly property bool modalOpen: playbackPanel.opened || settings.opened || queuePopup.opened || subtitleDialog.visible
    readonly property bool reveal: controlsShown || Player.paused || !Player.loaded || modalOpen
    function showControls() { controlsShown = true; hideTimer.restart() }
    function openSettings() { settings.open() }
    function closeSettings() { settings.close() }
    function time(n) {
        const t = Math.max(0, Math.floor(n || 0))
        return (t >= 3600 ? Math.floor(t / 3600) + ":" : "") + Math.floor(t % 3600 / 60).toString().padStart(2, "0") + ":" + (t % 60).toString().padStart(2, "0")
    }
    Timer { id: hideTimer; interval: 2800; running: hud.visible && Player.loaded && !Player.paused && !hud.modalOpen && !bottomHover.hovered && !topHover.hovered && !centerHover.hovered && hud.controlsShown; onTriggered: hud.controlsShown = false }
    Connections { target: Player; function onStateChanged() { if (Player.paused) hud.controlsShown = true } }
    MouseArea {
        anchors.fill: parent; hoverEnabled: true; acceptedButtons: Qt.LeftButton | Qt.RightButton
        onPositionChanged: hud.showControls()
        onClicked: mouse => { if (!bottomHover.hovered && !topHover.hovered && !centerHover.hovered) { if (mouse.button === Qt.RightButton) { playbackPanel.section = "more"; playbackPanel.open() } else Player.togglePause() } }
        onDoubleClicked: hud.fullscreenRequested()
        onWheel: wheel => { Player.setVolume(Player.volume + (wheel.angleDelta.y > 0 ? 5 : -5)); hud.showControls(); wheel.accepted = true }
    }
    DropArea { anchors.fill: parent; onDropped: drop => { if (drop.hasUrls && Player.openFiles(drop.urls, false)) drop.acceptProposedAction() } }
    Card {
        id: topBar
        anchors.top: parent.top; anchors.left: parent.left; anchors.right: parent.right; anchors.margins: 22
        height: 62; radius: 24
        color: Qt.rgba(Theme.surface.r, Theme.surface.g, Theme.surface.b, 0.94)
        opacity: hud.reveal ? 1 : 0; visible: opacity > 0; enabled: hud.reveal
        Behavior on opacity { NumberAnimation { duration: Theme.motion } }
        HoverHandler { id: topHover }
        RowLayout {
            anchors.fill: parent; anchors.margins: 9; spacing: 10
            JButton { compact: true; tone: "quiet"; iconName: "back"; hint: "Back to home"; onClicked: hud.backRequested() }
            Label { textFormat: Text.PlainText; text: Player.title.length ? Player.title : "Jadzumi Player"; font.bold: true; elide: Text.ElideMiddle; Layout.fillWidth: true }
            JButton { compact: true; tone: "quiet"; iconName: "queue"; hint: "Open queue"; onClicked: queuePopup.open() }
            JButton { compact: true; tone: "quiet"; iconName: "settings"; hint: "Settings"; onClicked: settings.open() }
        }
    }
    Card {
        id: centerBar
        anchors.centerIn: parent; width: 242; height: 88; radius: 44
        color: Qt.rgba(Theme.surface.r, Theme.surface.g, Theme.surface.b, 0.88)
        opacity: hud.reveal ? 1 : 0; visible: opacity > 0; enabled: Player.loaded && hud.reveal
        Behavior on opacity { NumberAnimation { duration: Theme.motion } }
        HoverHandler { id: centerHover }
        RowLayout {
            anchors.centerIn: parent; spacing: 16
            JButton { text: "−10"; compact: true; tone: "quiet"; hint: "Seek back ten seconds"; onClicked: Player.seekRelative(-10) }
            PlayPause {}
            JButton { text: "+10"; compact: true; tone: "quiet"; hint: "Seek forward ten seconds"; onClicked: Player.seekRelative(10) }
        }
    }
    Card {
        id: bottomBar
        width: Math.min(hud.width - 44, 1100); height: 124
        anchors.horizontalCenter: parent.horizontalCenter; anchors.bottom: parent.bottom; anchors.bottomMargin: 22
        color: Qt.rgba(Theme.surface.r, Theme.surface.g, Theme.surface.b, 0.95)
        opacity: hud.reveal ? 1 : 0; visible: opacity > 0; enabled: hud.reveal
        Behavior on opacity { NumberAnimation { duration: Theme.motion } }
        HoverHandler { id: bottomHover }
        ColumnLayout {
            anchors.fill: parent; anchors.margins: 17; spacing: 3
            Slider {
                id: seekBar
                Layout.fillWidth: true; implicitHeight: 20
                property bool scrubbing: false
                from: 0; to: Math.max(1, Player.duration); enabled: Player.loaded && Player.duration > 0
                Binding { target: seekBar; property: "value"; value: Player.position; when: !seekBar.scrubbing; restoreMode: Binding.RestoreNone }
                onMoved: { hud.showControls(); if (!pressed) Player.seek(value) }
                onPressedChanged: { if (pressed) scrubbing = true; else if (scrubbing) { Player.seek(value); scrubbing = false } }
                background: Rectangle {
                    x: seekBar.leftPadding; y: (seekBar.height - height) / 2
                    width: seekBar.availableWidth; height: 4; radius: 2; color: Theme.outline
                    Rectangle { width: seekBar.visualPosition * parent.width; height: parent.height; radius: 2; color: Theme.accent }
                }
                handle: Rectangle { x: seekBar.leftPadding + seekBar.visualPosition * (seekBar.availableWidth - width); y: (seekBar.height - height) / 2; width: seekBar.pressed ? 15 : 10; height: width; radius: width / 2; color: Theme.accent }
            }
            RowLayout {
                Layout.fillWidth: true; spacing: hud.width < 900 ? 3 : 9
                JButton { compact: true; tone: "quiet"; iconName: "previous"; hint: "Previous queue item"; enabled: Player.playlistIndex > 0; onClicked: Player.previous() }
                JButton { compact: true; tone: "quiet"; iconName: "next"; hint: "Next queue item"; enabled: Player.playlistIndex + 1 < Player.playlist.length; onClicked: Player.next() }
                Label { textFormat: Text.PlainText; text: hud.time(seekBar.scrubbing ? seekBar.value : Player.position) + " / " + hud.time(Player.duration); font.pixelSize: 12; color: Theme.muted; Layout.fillWidth: true }
                JButton { compact: true; tone: "quiet"; iconName: Player.muted ? "mute" : "volume"; hint: "Mute or unmute"; onClicked: Player.toggleMute() }
                Slider { Layout.preferredWidth: hud.width > 900 ? 90 : 58; from: 0; to: 100; value: Player.volume; onMoved: Player.setVolume(value); enabled: Player.ready }
                JButton { compact: true; tone: "quiet"; iconName: "audio"; hint: "Audio tracks and synchronization"; enabled: Player.loaded; onClicked: { playbackPanel.section = "audio"; playbackPanel.open() } }
                JButton { compact: true; tone: "quiet"; iconName: "subtitle"; hint: "Subtitles"; enabled: Player.loaded; onClicked: { playbackPanel.section = "subtitles"; playbackPanel.open() } }
                JButton { compact: true; tone: "quiet"; iconName: "screen"; hint: "Picture, speed and diagnostics"; enabled: Player.loaded; onClicked: { playbackPanel.section = "video"; playbackPanel.open() } }
                JButton { compact: true; tone: "quiet"; iconName: "more"; hint: "More playback tools"; enabled: Player.loaded; onClicked: { playbackPanel.section = "more"; playbackPanel.open() } }
                JButton { compact: true; tone: "quiet"; iconName: "fullscreen"; hint: "Toggle fullscreen"; onClicked: hud.fullscreenRequested() }
            }
        }
    }
    Card {
        visible: Player.error.length > 0
        anchors.horizontalCenter: parent.horizontalCenter; anchors.top: topBar.bottom; anchors.topMargin: 12
        width: Math.min(hud.width - 60, 780); height: errorRow.implicitHeight + 24
        RowLayout { id: errorRow; anchors.fill: parent; anchors.margins: 12; Label { textFormat: Text.PlainText; text: Player.error; color: Theme.danger; wrapMode: Text.Wrap; Layout.fillWidth: true } JButton { compact: true; tone: "quiet"; iconName: "close"; hint: "Dismiss error"; onClicked: Player.dismissError() } }
    }
    PlaybackPanel { id: playbackPanel; onSubtitleFileRequested: subtitleDialog.open() }
    SettingsPanel { id: settings }
    Popup {
        id: queuePopup; parent: Overlay.overlay; anchors.centerIn: parent
        width: Math.min(740, parent.width - 40); height: Math.min(620, parent.height - 40)
        modal: true; focus: true; padding: 24; background: Card {}
        contentItem: QueueView { compact: true; onAddRequested: { queuePopup.close(); hud.openRequested(true) } }
    }
    FileDialog { id: subtitleDialog; title: "Load local subtitles"; nameFilters: ["Subtitles (*.srt *.ass *.ssa *.vtt *.sub *.sup)"]; onAccepted: Player.loadSubtitle(selectedFile) }
}
