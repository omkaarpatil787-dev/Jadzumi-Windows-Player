import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Jadzumi
ScrollView {
    id: home
    signal openRequested(bool append)
    signal detailsRequested(var record)
    readonly property var records: Player.history.recent
    readonly property var continuing: records.filter(r => r.position >= 2 && r.duration - r.position > 3)
    clip: true; contentWidth: availableWidth
    function stamp(n) { const t = Math.max(0, Math.floor(n)); return Math.floor(t / 60) + ":" + (t % 60).toString().padStart(2, "0") }
    ColumnLayout {
        width: home.availableWidth; spacing: 26
        RowLayout {
            Label { textFormat: Text.PlainText; text: "Your cinema, at home."; font.pixelSize: 29 * Theme.scaleFactor; font.bold: true; Layout.fillWidth: true }
            Label { textFormat: Text.PlainText; text: "OFFLINE FIRST"; color: Theme.muted; font.pixelSize: 10; font.letterSpacing: 1.6 }
        }
        Card {
            Layout.fillWidth: true; implicitHeight: Math.max(250, heroColumn.implicitHeight + 64)
            clip: true; color: Theme.hero
            Rectangle { width: 310; height: 310; radius: 155; x: parent.width - 190; y: -130; color: Theme.accent; opacity: 0.09 }
            Rectangle { width: 220; height: 220; radius: 110; x: parent.width - 270; y: 120; color: Theme.accent; opacity: 0.05 }
            ColumnLayout {
                id: heroColumn; anchors.left: parent.left; anchors.right: parent.right; anchors.top: parent.top; anchors.margins: 32
                spacing: 16
                Label { textFormat: Text.PlainText; text: home.continuing.length ? "PICK UP WHERE YOU LEFT OFF" : "JADZUMI PLAYER"; color: Theme.accent; font.pixelSize: 11; font.letterSpacing: 1.6 }
                Label { textFormat: Text.PlainText; text: home.continuing.length ? home.continuing[0].title : "Your movies.\nYour space."; font.pixelSize: (home.continuing.length ? 29 : 36) * Theme.scaleFactor; font.bold: true; wrapMode: Text.Wrap; Layout.fillWidth: true; maximumLineCount: 2; elide: Text.ElideRight }
                Label { textFormat: Text.PlainText; text: home.continuing.length ? home.stamp(home.continuing[0].position) + " watched · " + home.stamp(home.continuing[0].duration - home.continuing[0].position) + " remaining" : "Open a local video and settle in. No accounts. No distractions."; color: Theme.muted; wrapMode: Text.Wrap; Layout.fillWidth: true }
                RowLayout {
                    JButton { text: home.continuing.length ? "Continue watching" : "Open a video"; tone: "primary"; iconName: "play"; enabled: Player.ready; onClicked: { if (home.continuing.length) home.detailsRequested(home.continuing[0]); else home.openRequested(false) } }
                    JButton { text: "Add to queue"; iconName: "plus"; tone: "quiet"; enabled: Player.ready; onClicked: home.openRequested(true) }
                }
            }
        }
        RowLayout {
            Label { textFormat: Text.PlainText; text: "Recently played"; font.pixelSize: 20 * Theme.scaleFactor; font.bold: true; Layout.fillWidth: true }
            Label { textFormat: Text.PlainText; text: home.records.length + " local " + (home.records.length === 1 ? "video" : "videos"); color: Theme.muted }
        }
        Card {
            visible: home.records.length === 0
            Layout.fillWidth: true; implicitHeight: 170
            ColumnLayout {
                anchors.centerIn: parent; spacing: 10
                Icon { name: "open"; tint: Theme.accent; Layout.preferredWidth: 30; Layout.preferredHeight: 30; Layout.alignment: Qt.AlignHCenter }
                Label { textFormat: Text.PlainText; text: "A little quiet in here."; font.pixelSize: 17; font.bold: true; Layout.alignment: Qt.AlignHCenter }
                Label { textFormat: Text.PlainText; text: "Open or drop a video. Your recent playback will appear here."; color: Theme.muted; wrapMode: Text.Wrap; Layout.maximumWidth: 380; horizontalAlignment: Text.AlignHCenter }
            }
        }
        GridLayout {
            Layout.fillWidth: true; columns: home.width > 1050 ? 3 : home.width > 550 ? 2 : 1; rowSpacing: 16; columnSpacing: 16
            Repeater {
                model: home.records.slice(0, 12)
                AbstractButton {
                    id: tile
                    required property var modelData
                    Layout.fillWidth: true; implicitHeight: 160; hoverEnabled: true
                    Accessible.name: "Details for " + modelData.title
                    background: Card { border.color: tile.hovered || tile.activeFocus ? Theme.accent : Theme.outline; color: tile.hovered ? Theme.raised : Theme.surface }
                    contentItem: ColumnLayout {
                        anchors.fill: parent; anchors.margins: 20; spacing: 12
                        RowLayout { Icon { name: "screen"; tint: Theme.accent } Item { Layout.fillWidth: true } Label { textFormat: Text.PlainText; text: home.stamp(tile.modelData.duration); color: Theme.muted; font.pixelSize: 12 } }
                        Label { textFormat: Text.PlainText; text: tile.modelData.title; font.bold: true; font.pixelSize: 16 * Theme.scaleFactor; elide: Text.ElideRight; Layout.fillWidth: true }
                        Label { textFormat: Text.PlainText; text: tile.modelData.position >= 2 ? "Resume from " + home.stamp(tile.modelData.position) : "Play from beginning"; color: Theme.muted; font.pixelSize: 12 }
                        Rectangle {
                            Layout.fillWidth: true; implicitHeight: 3; radius: 2; color: Theme.raised
                            Rectangle { width: parent.width * Math.min(1, tile.modelData.position / Math.max(1, tile.modelData.duration)); height: parent.height; color: Theme.accent; radius: 2 }
                        }
                    }
                    onClicked: home.detailsRequested(modelData)
                }
            }
        }
        Label { textFormat: Text.PlainText; text: "Drop local videos anywhere to play · Ctrl+O to open · Original frame rates preserved"; color: Theme.muted; font.pixelSize: 12; wrapMode: Text.Wrap; Layout.fillWidth: true }
        Item { implicitHeight: 16 }
    }
}
