import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Jadzumi
Popup {
    id: panel
    property string section: "audio"
    signal subtitleFileRequested()
    parent: Overlay.overlay; anchors.centerIn: parent
    width: Math.min(620, parent.width - 40); height: Math.min(610, parent.height - 40)
    modal: true; focus: true; padding: 24
    background: Card {}
    palette.windowText: Theme.text; palette.text: Theme.text; palette.buttonText: Theme.text
    palette.button: Theme.raised; palette.base: Theme.surface; palette.highlight: Theme.accent
    palette.highlightedText: Theme.onAccent
    enter: Transition { ParallelAnimation { NumberAnimation { property: "opacity"; from: 0; to: 1; duration: Theme.motion } NumberAnimation { property: "scale"; from: 0.96; to: 1; duration: Theme.motion; easing.type: Easing.OutCubic } } }
    function tracks(type, includeOff) {
        let result = includeOff ? [{label: "Off", id: -1, selected: false}] : []
        for (let t of Player.tracks) if (t.type === type) result.push({id: t.id, label: (t.title || (type === "audio" ? "Audio " : "Subtitle ") + t.id) + (t.lang ? " · " + t.lang : "") + (t.external ? " · external" : ""), selected: !!t.selected})
        return result
    }
    function selected(items) { for (let i = 0; i < items.length; ++i) if (items[i].selected) return i; return 0 }
    readonly property var audioItems: tracks("audio", false)
    readonly property var subtitleItems: tracks("sub", true)
    readonly property var rates: [0.5, 0.75, 1, 1.25, 1.5, 1.75, 2]
    contentItem: ColumnLayout {
        spacing: 18
        RowLayout {
            Label { textFormat: Text.PlainText; text: panel.section === "audio" ? "Audio Studio" : panel.section === "subtitles" ? "Subtitles" : panel.section === "video" ? "Picture & playback" : "Playback tools"; font.pixelSize: 23; font.bold: true; Layout.fillWidth: true }
            JButton { compact: true; tone: "quiet"; iconName: "close"; hint: "Close panel"; onClicked: panel.close() }
        }
        ScrollView {
            Layout.fillWidth: true; Layout.fillHeight: true; contentWidth: availableWidth; clip: true
            ColumnLayout {
                width: parent.width; spacing: 16
                ColumnLayout {
                    visible: panel.section === "audio"; Layout.fillWidth: true; spacing: 16
                    Label { textFormat: Text.PlainText; text: "Active audio track"; font.bold: true }
                    JCombo { Layout.fillWidth: true; model: panel.audioItems; textRole: "label"; currentIndex: panel.selected(panel.audioItems); enabled: panel.audioItems.length > 0; onActivated: Player.selectAudio(panel.audioItems[currentIndex].id) }
                    Label { textFormat: Text.PlainText; text: (Player.diagnostics["audio-codec-name"] || "Codec not reported") + "\n" + JSON.stringify(Player.diagnostics["audio-params"] || {}, null, 2); color: Theme.muted; wrapMode: Text.WrapAnywhere; Layout.fillWidth: true }
                    Label { textFormat: Text.PlainText; text: "Audio synchronization"; font.bold: true }
                    RowLayout {
                        JButton { text: "−0.1 s"; onClicked: Player.setAudioDelay(Player.audioDelay - 0.1) }
                        Label { textFormat: Text.PlainText; text: Player.audioDelay.toFixed(1) + " s"; horizontalAlignment: Text.AlignHCenter; Layout.fillWidth: true }
                        JButton { text: "+0.1 s"; onClicked: Player.setAudioDelay(Player.audioDelay + 0.1) }
                    }
                    JButton { text: "Reset synchronization"; onClicked: Player.setAudioDelay(0) }
                    Label { textFormat: Text.PlainText; text: "Output backend: " + (Player.diagnostics["current-ao"] || "Not reported") + "\nDevice routing, passthrough and audio processing are outside this M2 build."; color: Theme.muted; wrapMode: Text.Wrap; Layout.fillWidth: true }
                }
                ColumnLayout {
                    visible: panel.section === "subtitles"; Layout.fillWidth: true; spacing: 16
                    JCombo { Layout.fillWidth: true; model: panel.subtitleItems; textRole: "label"; currentIndex: panel.selected(panel.subtitleItems); onActivated: Player.selectSubtitle(panel.subtitleItems[currentIndex].id) }
                    CheckBox { text: "Show subtitles"; checked: Player.subtitlesVisible; onToggled: Player.toggleSubtitles() }
                    JButton { text: "Load subtitle file"; iconName: "open"; onClicked: panel.subtitleFileRequested() }
                    Label { textFormat: Text.PlainText; text: "Subtitle synchronization"; font.bold: true }
                    RowLayout {
                        JButton { text: "−0.1 s"; onClicked: Player.setSubtitleDelay(Player.subtitleDelay - 0.1) }
                        Label { textFormat: Text.PlainText; text: Player.subtitleDelay.toFixed(1) + " s"; horizontalAlignment: Text.AlignHCenter; Layout.fillWidth: true }
                        JButton { text: "+0.1 s"; onClicked: Player.setSubtitleDelay(Player.subtitleDelay + 0.1) }
                    }
                    Label { textFormat: Text.PlainText; text: "Size · " + Math.round(Player.subtitleScale * 100) + "%" }
                    Slider { Layout.fillWidth: true; from: 0.5; to: 3; stepSize: 0.1; value: Player.subtitleScale; onMoved: Player.setSubtitleScale(value) }
                    Label { textFormat: Text.PlainText; text: "Vertical position · " + Math.round(Player.subtitlePosition) + "%" }
                    Slider { Layout.fillWidth: true; from: 0; to: 150; stepSize: 1; value: Player.subtitlePosition; onMoved: Player.setSubtitlePosition(value) }
                    JButton { text: "Reset subtitle adjustments"; onClicked: { Player.setSubtitleDelay(0); Player.setSubtitleScale(1); Player.setSubtitlePosition(100) } }
                    Label { textFormat: Text.PlainText; text: "ASS/SSA styling is rendered by libmpv/libass. Scripted positioning can take precedence over these adjustments."; color: Theme.muted; wrapMode: Text.Wrap; Layout.fillWidth: true }
                }
                ColumnLayout {
                    visible: panel.section === "video"; Layout.fillWidth: true; spacing: 16
                    RowLayout { Label { textFormat: Text.PlainText; text: "Playback speed"; Layout.fillWidth: true } JCombo { model: ["0.5×", "0.75×", "1×", "1.25×", "1.5×", "1.75×", "2×"]; currentIndex: panel.rates.indexOf(Player.speed); onActivated: Player.setSpeed(panel.rates[currentIndex]) } }
                    RowLayout { Label { textFormat: Text.PlainText; text: "Aspect ratio"; Layout.fillWidth: true } JCombo { model: ["Auto", "16:9", "16:10", "4:3", "21:9"]; currentIndex: model.indexOf(Player.aspect); onActivated: Player.setAspect(model[currentIndex]) } }
                    RowLayout { Label { textFormat: Text.PlainText; text: "Scaling"; Layout.fillWidth: true } JCombo { model: ["Fit", "Fill", "Zoom"]; currentIndex: model.indexOf(Player.scaleMode); onActivated: Player.setScaleMode(model[currentIndex]) } }
                    RowLayout { Label { textFormat: Text.PlainText; text: "Rotation"; Layout.fillWidth: true } JCombo { model: ["0°", "90°", "180°", "270°"]; currentIndex: Player.rotation / 90; onActivated: Player.setRotation(currentIndex * 90) } }
                    Label { textFormat: Text.PlainText; text: "Decoder preference"; font.bold: true }
                    JCombo { Layout.fillWidth: true; model: ["D3D11VA with software fallback", "Software decoding"]; onActivated: Player.setHardwareDecoding(currentIndex === 0) }
                    Label { textFormat: Text.PlainText; text: "Observed decoder: " + (Player.diagnostics["hwdec-current"] || "Not reported") + "\nSDR output · HDR output verification is pending."; color: Theme.muted; wrapMode: Text.Wrap; Layout.fillWidth: true }
                    TextArea { textFormat: TextEdit.PlainText; Layout.fillWidth: true; readOnly: true; selectByMouse: true; wrapMode: TextEdit.WrapAnywhere; text: JSON.stringify(Player.diagnostics, null, 2); font.pixelSize: 11; color: Theme.muted; background: Rectangle { color: "transparent" } }
                }
                ColumnLayout {
                    visible: panel.section === "more"; Layout.fillWidth: true; spacing: 16
                    CheckBox { text: "Loop this video"; checked: Player.looping; onToggled: Player.setLooping(checked) }
                    RowLayout { JButton { text: "Set A"; onClicked: Player.setAbStart() } JButton { text: "Set B"; onClicked: Player.setAbEnd() } JButton { text: "Clear A–B"; onClicked: Player.clearAb() } }
                    Label { textFormat: Text.PlainText; text: "A: " + Player.abStart + " · B: " + Player.abEnd; color: Theme.muted }
                    JButton { text: "Step one frame"; iconName: "next"; onClicked: Player.frameStep() }
                    JButton { text: "Capture screenshot"; iconName: "camera"; onClicked: Player.screenshot() }
                    Label { textFormat: Text.PlainText; text: "Chapters"; font.bold: true }
                    Label { textFormat: Text.PlainText; visible: Player.chapters.length === 0; text: "No chapters in this file."; color: Theme.muted }
                    Repeater {
                        model: Player.chapters
                        JButton { required property var modelData; required property int index; Layout.fillWidth: true; text: (index + 1) + ". " + (modelData.title || "Chapter " + (index + 1)); tone: index === Player.chapterIndex ? "primary" : "quiet"; onClicked: Player.setChapter(index) }
                    }
                    JButton { text: "Stop playback"; iconName: "stop"; onClicked: { Player.stop(); panel.close() } }
                }
                Item { implicitHeight: 8; Layout.fillWidth: true }
            }
        }
    }
}
