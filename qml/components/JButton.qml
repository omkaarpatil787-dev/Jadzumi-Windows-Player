import QtQuick
import Jadzumi
import QtQuick.Controls
import QtQuick.Layouts
Button {
    id: button
    property string iconName: ""
    property string tone: "tonal"
    property string hint: ""
    property bool compact: false
    property color foreground: tone === "primary" ? Theme.onAccent : Theme.text
    implicitHeight: 44 * Theme.scaleFactor
    implicitWidth: Math.max(compact ? implicitHeight : 72, contents.implicitWidth + (compact ? 20 : 30))
    font.family: Theme.fontFamily; font.pixelSize: 14 * Theme.scaleFactor
    hoverEnabled: true
    opacity: enabled ? 1 : 0.42
    scale: down ? 0.95 : 1
    Behavior on scale { NumberAnimation { duration: Theme.motion / 2; easing.type: Easing.OutCubic } }
    contentItem: RowLayout {
        id: contents
        spacing: button.text.length ? 9 : 0
        Icon { visible: button.iconName.length > 0; name: button.iconName; tint: button.foreground; Layout.preferredWidth: 20; Layout.preferredHeight: 20; Layout.alignment: Qt.AlignVCenter }
        Text { textFormat: Text.PlainText; visible: button.text.length > 0; text: button.text; color: button.foreground; font: button.font; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; Layout.fillWidth: true }
    }
    background: Rectangle {
        radius: button.compact ? height / 2 : 16
        color: button.tone === "primary" ? Theme.accent : button.tone === "quiet" ? (button.hovered ? Theme.raised : "transparent") : Theme.raised
        border.color: button.activeFocus ? Theme.accent : "transparent"
        border.width: button.activeFocus ? 2 : 0
        Rectangle { anchors.fill: parent; radius: parent.radius; color: button.foreground; opacity: button.down ? 0.12 : button.hovered ? 0.055 : 0 }
    }
    ToolTip.visible: hovered && hint.length > 0
    ToolTip.text: hint
    ToolTip.delay: 600
    Accessible.name: text.length ? text : hint
}
