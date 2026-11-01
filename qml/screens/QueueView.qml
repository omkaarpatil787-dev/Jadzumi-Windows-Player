import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Jadzumi
ColumnLayout {
    id: queue
    signal addRequested()
    property bool compact: false
    spacing: 18
    RowLayout {
        Label { textFormat: Text.PlainText; text: "Your queue"; font.pixelSize: queue.compact ? 22 : 29; font.bold: true; Layout.fillWidth: true }
        JButton { text: "Add videos"; iconName: "plus"; enabled: Player.ready; onClicked: queue.addRequested() }
        JButton { text: "Keep current only"; visible: Player.playlist.length > 1 && !queue.compact; onClicked: Player.clearQueue() }
    }
    Label { textFormat: Text.PlainText; text: Player.playlist.length + " " + (Player.playlist.length === 1 ? "video" : "videos") + " · Files stay exactly where they are."; color: Theme.muted }
    Label { textFormat: Text.PlainText; visible: Player.playlist.length === 0; text: "Choose local videos to start your queue."; color: Theme.muted; Layout.topMargin: 32 }
    ListView {
        Layout.fillWidth: true; Layout.fillHeight: true; model: Player.playlist; clip: true; spacing: 10
        ScrollBar.vertical: ScrollBar {}
        delegate: Card {
            id: entry
            required property var modelData
            required property int index
            width: ListView.view.width; height: 76
            border.color: index === Player.playlistIndex ? Theme.accent : Theme.outline
            RowLayout {
                anchors.fill: parent; anchors.margins: 12; spacing: 12
                JButton { compact: true; tone: entry.index === Player.playlistIndex ? "primary" : "quiet"; iconName: "play"; hint: "Play this queue item"; onClicked: Player.playIndex(entry.index) }
                ColumnLayout {
                    Layout.fillWidth: true; spacing: 3
                    Label { textFormat: Text.PlainText; text: entry.modelData.title || entry.modelData.filename.replace(/\\/g, "/").split("/").pop(); font.bold: true; elide: Text.ElideRight; Layout.fillWidth: true }
                    Label { textFormat: Text.PlainText; text: entry.index === Player.playlistIndex ? "Now playing" : "Queued"; color: Theme.muted; font.pixelSize: 11 }
                }
                JButton { compact: true; tone: "quiet"; iconName: "close"; hint: "Remove from queue"; onClicked: Player.removeQueueItem(entry.index) }
            }
        }
    }
}
