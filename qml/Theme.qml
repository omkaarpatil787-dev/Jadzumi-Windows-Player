pragma Singleton
import QtQuick

Item {
    id: theme
    readonly property bool dark: Preferences.theme === "dark" || Preferences.theme === "amoled" ||
                                 (Preferences.theme === "system" && systemPalette.window.hslLightness < 0.5)
    readonly property bool black: Preferences.theme === "amoled"
    readonly property real scaleFactor: Preferences.uiScale
    readonly property int motion: Preferences.effectiveReducedMotion ? 0 : 200
    readonly property string fontFamily: Qt.platform.os === "windows" ? "Segoe UI Variable" : "Sans Serif"
    property color canvas: dark ? (black ? "#000000" : "#101722") : "#f2f6fc"
    property color surface: dark ? (black ? "#0b0d12" : "#192332") : "#ffffff"
    property color raised: dark ? "#233247" : "#e8f0fc"
    property color text: dark ? "#eaf1fc" : "#16263c"
    property color muted: dark ? "#a8b8ce" : "#5d7089"
    property color outline: dark ? "#344055" : "#d6e0ee"
    property color accent: dark ? "#a4cdff" : "#235fa7"
    property color onAccent: dark ? "#102f53" : "#ffffff"
    property color danger: dark ? "#ffb5c0" : "#a52e44"
    property SystemPalette systemPalette: SystemPalette { colorGroup: SystemPalette.Active }
    Behavior on canvas { ColorAnimation { duration: theme.motion } }
    Behavior on surface { ColorAnimation { duration: theme.motion } }
    Behavior on raised { ColorAnimation { duration: theme.motion } }
    Behavior on text { ColorAnimation { duration: theme.motion } }
    Behavior on muted { ColorAnimation { duration: theme.motion } }
    Behavior on outline { ColorAnimation { duration: theme.motion } }
    Behavior on accent { ColorAnimation { duration: theme.motion } }
    Behavior on onAccent { ColorAnimation { duration: theme.motion } }
}
