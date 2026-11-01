import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Jadzumi
Popup {
    id: details
    property var record: ({})
    signal playRequested(string path, bool fromBeginning)
    parent: Overlay.overlay; anchors.centerIn: parent
    width: Math.min(620, parent.width - 40); padding: 28
    modal: true; focus: true
    background: Card {}
    enter: Transition { ParallelAnimation { NumberAnimation { property: "opacity"; from: 0; to: 1; duration: Theme.motion } NumberAnimation { property: "scale"; from: 0.96; to: 1; duration: Theme.motion; easing.type: Easing.OutCubic } } }
    contentItem: ColumnLayout {
        spacing: 20
        RowLayout { Icon { name: "screen"; tint: Theme.accent; Layout.preferredWidth: 32; Layout.preferredHeight: 32 } Item { Layout.fillWidth: true } JButton { compact: true; tone: "quiet"; iconName: "close"; hint: "Close details"; onClicked: details.close() } }
        Label { textFormat: Text.PlainText; text: details.record.title || "Local video"; font.pixelSize: 26; font.bold: true; wrapMode: Text.Wrap; Layout.fillWidth: true }
        Label { textFormat: Text.PlainText; text: Math.round((details.record.duration || 0) / 60) + " minutes · " + ((details.record.size || 0) / (1024 * 1024)).toFixed(1) + " MB"; color: Theme.muted }
        Label { textFormat: Text.PlainText; text: details.record.position >= 2 ? "Resume from " + Math.floor(details.record.position / 60) + ":" + Math.floor(details.record.position % 60).toString().padStart(2, "0") : "Ready to play from the beginning"; color: Theme.muted }
        TextArea { textFormat: TextEdit.PlainText; Layout.fillWidth: true; readOnly: true; selectByMouse: true; wrapMode: TextEdit.WrapAnywhere; text: details.record.path || ""; color: Theme.muted; font.pixelSize: 12; background: Rectangle { color: "transparent" } }
        RowLayout {
            JButton { text: details.record.position >= 2 ? "Resume" : "Play"; iconName: "play"; tone: "primary"; onClicked: { details.playRequested(details.record.path, false); details.close() } }
            JButton { text: "Play from beginning"; onClicked: { details.playRequested(details.record.path, true); details.close() } }
        }
    }
}
