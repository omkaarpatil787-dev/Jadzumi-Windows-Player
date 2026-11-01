pragma Singleton
import QtQuick

QtObject {
    readonly property bool dark: ThemeState.dark
    readonly property bool black: ThemeState.black
    readonly property real scaleFactor: ThemeState.scaleFactor
    readonly property int motion: ThemeState.motion
    readonly property string fontFamily: ThemeState.fontFamily
    readonly property color canvas: ThemeState.canvas
    readonly property color surface: ThemeState.surface
    readonly property color raised: ThemeState.raised
    readonly property color text: ThemeState.text
    readonly property color muted: ThemeState.muted
    readonly property color outline: ThemeState.outline
    readonly property color accent: ThemeState.accent
    readonly property color accentInk: ThemeState.accentInk
    readonly property color danger: ThemeState.danger
    readonly property color hero: ThemeState.hero
}
