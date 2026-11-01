import QtQuick
import Jadzumi
import QtQuick.Controls
import QtQuick.Shapes
AbstractButton {
    id: control
    implicitWidth: 64; implicitHeight: 64
    property real morph: Player.paused ? 0 : 1
    Behavior on morph { NumberAnimation { duration: Theme.motion; easing.type: Easing.OutCubic } }
    opacity: enabled ? 1 : 0.4
    hoverEnabled: true
    scale: down ? 0.92 : hovered ? 1.04 : 1
    Behavior on scale { NumberAnimation { duration: Theme.motion; easing.type: Easing.OutCubic } }
    background: Rectangle { radius: height / 2; color: Theme.accent }
    contentItem: Item {
        Item {
            width: 24; height: 24; anchors.centerIn: parent
            Shape {
                anchors.fill: parent
                ShapePath {
                    fillColor: Theme.onAccent; strokeColor: "transparent"
                    PathMove { x: 7 - control.morph; y: 4 }
                    PathLine { x: 12 - 2 * control.morph; y: 7.6 - 3.6 * control.morph }
                    PathLine { x: 12 - 2 * control.morph; y: 16.4 + 3.6 * control.morph }
                    PathLine { x: 7 - control.morph; y: 20 }
                    PathLine { x: 7 - control.morph; y: 4 }
                }
                ShapePath {
                    fillColor: Theme.onAccent; strokeColor: "transparent"
                    PathMove { x: 12 + 2 * control.morph; y: 7.6 - 3.6 * control.morph }
                    PathLine { x: 19 - control.morph; y: 12 - 8 * control.morph }
                    PathLine { x: 19 - control.morph; y: 12 + 8 * control.morph }
                    PathLine { x: 12 + 2 * control.morph; y: 16.4 + 3.6 * control.morph }
                    PathLine { x: 12 + 2 * control.morph; y: 7.6 - 3.6 * control.morph }
                }
            }
        }
    }
    Accessible.name: Player.paused ? "Play" : "Pause"
    onClicked: Player.togglePause()
}
