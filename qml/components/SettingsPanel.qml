import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Jadzumi
Popup {
    id: panel
    parent: Overlay.overlay
    anchors.centerIn: parent
    width: Math.min(720, parent.width - 40)
    height: Math.min(700, parent.height - 40)
    padding: 24; modal: true; focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    palette.windowText: Theme.text; palette.text: Theme.text; palette.base: Theme.surface
    palette.buttonText: Theme.text; palette.button: Theme.raised; palette.highlight: Theme.accent
    palette.highlightedText: Theme.accentInk
    background: Card {}
    enter: Transition { ParallelAnimation { NumberAnimation { property: "opacity"; from: 0; to: 1; duration: Theme.motion } NumberAnimation { property: "scale"; from: 0.97; to: 1; duration: Theme.motion; easing.type: Easing.OutCubic } } }
    exit: Transition { NumberAnimation { property: "opacity"; to: 0; duration: Theme.motion / 2 } }
    contentItem: ColumnLayout {
        spacing: 16
        RowLayout {
            Label { textFormat: Text.PlainText; text: "Make it yours"; font.pixelSize: 25 * Theme.scaleFactor; font.bold: true; Layout.fillWidth: true }
            JButton { compact: true; tone: "quiet"; iconName: "close"; hint: "Close settings"; onClicked: panel.close() }
        }
        ScrollView {
            Layout.fillWidth: true; Layout.fillHeight: true; clip: true
            contentWidth: availableWidth
            ColumnLayout {
                width: parent.width; spacing: 18
                Label { textFormat: Text.PlainText; text: "APPEARANCE"; color: Theme.muted; font.pixelSize: 11; font.letterSpacing: 1.5 }
                RowLayout {
                    Label { textFormat: Text.PlainText; text: "Theme"; Layout.fillWidth: true }
                    JCombo { model: ["Light", "Dark", "AMOLED", "System"]; currentIndex: ["light", "dark", "amoled", "system"].indexOf(Preferences.theme); onActivated: Preferences.theme = ["light", "dark", "amoled", "system"][currentIndex] }
                }
                CheckBox { text: "Animate navigation and theme changes"; checked: Preferences.animations; onToggled: Preferences.animations = checked }
                CheckBox { text: "Reduce motion"; checked: Preferences.reducedMotion; onToggled: Preferences.reducedMotion = checked }
                Label { textFormat: Text.PlainText; text: Preferences.effectiveReducedMotion ? "Reduced motion is active. Windows animation preferences are also respected." : "Natural easing, brief transitions and no continuous background effects."; color: Theme.muted; wrapMode: Text.Wrap; Layout.fillWidth: true }
                RowLayout {
                    Label { textFormat: Text.PlainText; text: "Interface scale"; Layout.fillWidth: true }
                    Slider { from: 0.85; to: 1.25; stepSize: 0.05; value: Preferences.uiScale; onMoved: Preferences.uiScale = value; Layout.preferredWidth: 180 }
                    Label { textFormat: Text.PlainText; text: Math.round(Preferences.uiScale * 100) + "%"; Layout.preferredWidth: 45 }
                }
                CheckBox { text: "Expanded sidebar on wide windows"; checked: Preferences.expandedSidebar; onToggled: Preferences.expandedSidebar = checked }
                Rectangle { Layout.fillWidth: true; implicitHeight: 1; color: Theme.outline }
                Label { textFormat: Text.PlainText; text: "PLAYBACK"; color: Theme.muted; font.pixelSize: 11; font.letterSpacing: 1.5 }
                CheckBox { text: "Resume videos from the last position"; checked: Preferences.resumeEnabled; onToggled: Preferences.resumeEnabled = checked }
                CheckBox { text: "Automatically play the next queue item"; checked: Preferences.autoplay; onToggled: Preferences.autoplay = checked }
                RowLayout {
                    Label { textFormat: Text.PlainText; text: "Saved playback history"; Layout.fillWidth: true }
                    JButton { text: "Clear history"; onClicked: { Player.history.clear(); historyCleared.visible = true } }
                }
                Label { textFormat: Text.PlainText; id: historyCleared; visible: false; text: "Saved entries cleared. Active playback can create a new resume entry."; color: Theme.muted; wrapMode: Text.Wrap; Layout.fillWidth: true }
                Label { textFormat: Text.PlainText; visible: Player.history.error.length > 0; text: Player.history.error; color: Theme.danger; wrapMode: Text.Wrap; Layout.fillWidth: true }
                Rectangle { Layout.fillWidth: true; implicitHeight: 1; color: Theme.outline }
                Label { textFormat: Text.PlainText; text: "KEYBOARD SHORTCUTS"; color: Theme.muted; font.pixelSize: 11; font.letterSpacing: 1.5 }
                Label { textFormat: Text.PlainText; text: "Enter a key combination such as Ctrl+P. Shortcuts pause while editing. Esc, F11 and Alt+F4 remain reserved."; color: Theme.muted; wrapMode: Text.Wrap; Layout.fillWidth: true }
                Repeater {
                    model: Object.keys(Preferences.shortcuts)
                    RowLayout {
                        required property string modelData
                        Layout.fillWidth: true
                        Label { textFormat: Text.PlainText; text: modelData.replace(/([A-Z])/g, " $1"); Layout.fillWidth: true }
                        TextField {
                            text: Preferences.shortcuts[parent.modelData]; Layout.preferredWidth: 180
                            selectByMouse: true
                            onEditingFinished: { Preferences.setShortcut(parent.modelData, text); text = Preferences.shortcuts[parent.modelData] }
                        }
                    }
                }
                Label { textFormat: Text.PlainText; text: Preferences.shortcutError; visible: text.length > 0; color: Theme.danger; wrapMode: Text.Wrap; Layout.fillWidth: true }
                JButton { text: "Restore default shortcuts"; onClicked: Preferences.resetShortcuts() }
                Rectangle { Layout.fillWidth: true; implicitHeight: 1; color: Theme.outline }
                Label { textFormat: Text.PlainText; text: "YOUR MOVIES. YOUR SPACE."; font.pixelSize: 17; font.bold: true }
                Label { textFormat: Text.PlainText; text: "Local playback · No analytics · No account required\nCreated by Omkar Patil · AI-assisted development\nVersion " + Qt.application.version + " · Windows x64 development build\nOriginal video timing. No motion interpolation."; color: Theme.muted; wrapMode: Text.Wrap; Layout.fillWidth: true }
                Label { textFormat: Text.PlainText; text: "HDR output and target-GPU performance still require verification. This build uses SDR output. Dependency notices are included in the portable folder.\n\nNo popcorn included. Sorry."; color: Theme.muted; wrapMode: Text.Wrap; Layout.fillWidth: true }
                Item { implicitHeight: 8; Layout.fillWidth: true }
            }
        }
    }
}
