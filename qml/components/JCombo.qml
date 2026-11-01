import QtQuick
import Jadzumi
import QtQuick.Controls
ComboBox {
    id: control
    implicitHeight: 44 * Theme.scaleFactor
    implicitWidth: 160
    font.family: Theme.fontFamily; font.pixelSize: 14 * Theme.scaleFactor
    palette.text: Theme.text; palette.buttonText: Theme.text; palette.highlight: Theme.accent
    palette.highlightedText: Theme.accentInk; palette.window: Theme.surface
    leftPadding: 14; rightPadding: 34
    contentItem: Text { textFormat: Text.PlainText; text: control.displayText; font: control.font; color: control.enabled ? Theme.text : Theme.muted; verticalAlignment: Text.AlignVCenter; elide: Text.ElideRight }
    background: Rectangle { color: Theme.raised; radius: 13; border.color: control.activeFocus ? Theme.accent : Theme.outline; border.width: 1 }
    indicator: Icon { name: "down"; tint: Theme.muted; x: control.width - 28; y: (control.height - height) / 2; width: 18; height: 18 }
    popup: Popup {
        y: control.height + 4; width: control.width; padding: 5
        implicitHeight: Math.min(contentItem.implicitHeight + 10, 300)
        contentItem: ListView {
            clip: true; implicitHeight: contentHeight
            model: control.popup.visible ? control.delegateModel : null
            currentIndex: control.highlightedIndex
            ScrollIndicator.vertical: ScrollIndicator {}
        }
        background: Rectangle { color: Theme.surface; radius: 14; border.color: Theme.outline }
    }
    delegate: ItemDelegate {
        id: rowDelegate
        required property int index
        width: control.width - 10
        text: control.textRole ? (control.model[index][control.textRole] || "") : control.model[index]
        font: control.font
        highlighted: control.highlightedIndex === index
        contentItem: Text { textFormat: Text.PlainText; text: rowDelegate.text; font: control.font; color: Theme.text; verticalAlignment: Text.AlignVCenter; elide: Text.ElideRight }
        background: Rectangle { radius: 10; color: rowDelegate.highlighted ? Theme.raised : "transparent" }
    }
}
