import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

ApplicationWindow {
    id: root
    visible: true
    width: 1100; height: 760
    minimumWidth: 720; minimumHeight: 480
    title: (Player.title.length ? Player.title + " — " : "") + "Jadzumi Player · M0"
    color: "#101724"
    palette.window: "#101724"
    palette.windowText: "#e8f0ff"
    palette.text: "#e8f0ff"
    palette.button: "#253449"
    palette.buttonText: "#e8f0ff"
    palette.highlight: "#9bcaff"
    property bool diagnosticsVisible: false
    function fullscreen() {
        if (visibility === Window.FullScreen) showNormal()
        else showFullScreen()
    }
    function time(seconds) {
        const value = Math.max(0, Math.floor(seconds || 0))
        return Math.floor(value / 3600).toString().padStart(2, "0") + ":" +
               Math.floor(value % 3600 / 60).toString().padStart(2, "0") + ":" +
               (value % 60).toString().padStart(2, "0")
    }
    FileDialog {
        id: openDialog
        title: "Open a local video"
        nameFilters: ["Video files (*.mkv *.mp4 *.mov *.avi *.webm *.ts *.mts *.m2ts *.m4v *.flv *.mpg *.mpeg)"]
        onAccepted: Player.open(selectedFile)
    }
    Shortcut { sequence: "Ctrl+O"; onActivated: openDialog.open() }
    Shortcut { sequence: "Space"; onActivated: Player.togglePause() }
    Shortcut { sequence: "Left"; onActivated: Player.seekRelative(-10) }
    Shortcut { sequence: "Right"; onActivated: Player.seekRelative(10) }
    Shortcut { sequence: "Up"; onActivated: Player.setVolume(Player.volume + 5) }
    Shortcut { sequence: "Down"; onActivated: Player.setVolume(Player.volume - 5) }
    Shortcut { sequence: "M"; onActivated: Player.toggleMute() }
    Shortcut { sequence: "F"; onActivated: root.fullscreen() }
    Shortcut { sequence: "F11"; onActivated: root.fullscreen() }
    Shortcut { sequence: "Escape"; onActivated: { if (root.visibility === Window.FullScreen) root.showNormal() } }
    ColumnLayout {
        anchors.fill: parent
        spacing: 0
        RowLayout {
            Layout.fillWidth: true; Layout.margins: 12
            Label { text: "JADZUMI PLAYER"; font.bold: true; font.pixelSize: 17 }
            Label { text: "M0 · Playback foundation"; color: "#a5b5cd"; Layout.fillWidth: true }
            Button { text: "Open video"; enabled: Player.ready; onClicked: openDialog.open() }
            Button { text: "Diagnostics"; checkable: true; checked: root.diagnosticsVisible; onClicked: root.diagnosticsVisible = checked }
        }
        Rectangle {
            color: "#2e2230"; visible: Player.error.length > 0
            Layout.fillWidth: true; implicitHeight: errorLabel.implicitHeight + 24
            Label { id: errorLabel; anchors.fill: parent; anchors.margins: 12; text: Player.error; wrapMode: Text.Wrap; color: "#ffd3da" }
        }
        RowLayout {
            Layout.fillWidth: true; Layout.fillHeight: true; spacing: 0
            // Native HWNDs occupy their own stacking layer. No QML overlay is placed here in M0.
            WindowContainer { Layout.fillWidth: true; Layout.fillHeight: true; window: VideoSurface }
            Rectangle {
                visible: root.diagnosticsVisible
                Layout.preferredWidth: 300; Layout.fillHeight: true; color: "#192234"
                ScrollView {
                    anchors.fill: parent; anchors.margins: 12
                    TextArea {
                        readOnly: true; selectByMouse: true; wrapMode: TextEdit.WrapAnywhere
                        text: "Observed libmpv properties\n\n" + JSON.stringify(Player.diagnostics, null, 2) +
                              "\n\nMissing values are unavailable.\n\nOutput policy: SDR (BT.709). HDR output is not validated in M0.\n\nCPU/GPU utilization is not measured."
                        font.pixelSize: 12; color: "#dce8fa"
                        background: Rectangle { color: "transparent" }
                    }
                }
            }
        }
        ColumnLayout {
            Layout.fillWidth: true; Layout.margins: 12; spacing: 5
            Label {
                text: Player.loaded ? Player.title : "Open a local video to begin · Original video timing · No interpolation"
                elide: Text.ElideMiddle; Layout.fillWidth: true
            }
            Slider {
                id: progress
                property bool scrubbing: false
                Layout.fillWidth: true; from: 0; to: Math.max(1, Player.duration)
                enabled: Player.loaded && Player.duration > 0
                Binding { target: progress; property: "value"; value: Player.position; when: !progress.scrubbing; restoreMode: Binding.RestoreNone }
                onMoved: { if (!pressed && enabled) Player.seek(value) }
                onPressedChanged: {
                    if (pressed) scrubbing = true
                    else if (scrubbing) { if (enabled) Player.seek(value); scrubbing = false }
                }
            }
            RowLayout {
                Layout.fillWidth: true
                Button { text: Player.paused ? "Play" : "Pause"; enabled: Player.loaded; onClicked: Player.togglePause() }
                Button { text: "Stop"; enabled: Player.loaded; onClicked: Player.stop() }
                Button { text: "−10 s"; enabled: Player.loaded; onClicked: Player.seekRelative(-10) }
                Button { text: "+10 s"; enabled: Player.loaded; onClicked: Player.seekRelative(10) }
                Label { text: root.time(Player.position) + " / " + root.time(Player.duration); Layout.fillWidth: true }
                Button { text: Player.muted ? "Unmute" : "Mute"; enabled: Player.ready; onClicked: Player.toggleMute() }
                Slider { Layout.preferredWidth: 100; from: 0; to: 100; value: Player.volume; enabled: Player.ready; onMoved: Player.setVolume(value) }
                Button { text: "Fullscreen"; onClicked: root.fullscreen() }
            }
            RowLayout {
                Label { text: "Decoder:"; color: "#a5b5cd" }
                ComboBox {
                    model: ["Automatic D3D11VA + software fallback", "Software (recovery)"]
                    enabled: Player.ready
                    onActivated: Player.setHardwareDecoding(currentIndex === 0)
                }
                Label { text: Player.diagnostics["hwdec-current"] || "Not reported"; color: "#a5b5cd"; Layout.fillWidth: true }
            }
        }
    }
}
