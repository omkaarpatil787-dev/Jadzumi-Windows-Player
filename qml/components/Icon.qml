import QtQuick
import Jadzumi
import QtQuick.Shapes
Item {
    id: icon
    property string name: "play"
    property color tint: Theme.text
    implicitWidth: 24; implicitHeight: 24
    readonly property var paths: ({
        home: "M3 11L12 3L21 11 M5 10V21H10V15H14V21H19V10",
        menu: "M4 6H20 M4 12H20 M4 18H20",
        open: "M3 19V5H10L13 8H21V19Z M3 19L7 11H22L18 19Z",
        queue: "M4 5H17 M4 10H17 M4 15H12 M17 15L22 18L17 21Z",
        clock: "M12 3A9 9 0 1 0 12 21A9 9 0 1 0 12 3 M12 7V12L16 14",
        settings: "M9 3H15L16 6L19 7L22 11L20 14L20 18L16 20L13 22L9 20L5 19L3 15L4 11L4 7L8 6Z M12 8A4 4 0 1 0 12 16A4 4 0 1 0 12 8",
        back: "M14 5L7 12L14 19 M7 12H21",
        close: "M6 6L18 18 M18 6L6 18",
        play: "M7 4L20 12L7 20Z",
        pause: "M7 4V20 M17 4V20",
        stop: "M6 6H18V18H6Z",
        next: "M5 5L15 12L5 19Z M19 5V19",
        previous: "M19 5L9 12L19 19Z M5 5V19",
        volume: "M3 9H7L12 5V19L7 15H3Z M16 8Q21 12 16 16 M19 4Q27 12 19 20",
        mute: "M3 9H7L12 5V19L7 15H3Z M16 9L22 15 M22 9L16 15",
        subtitle: "M3 5H21V19H3Z M6 10H9 M12 10H18 M6 14H12 M15 14H18",
        audio: "M9 4V16 M9 5L20 3V14 M9 15Q3 12 3 17Q3 21 9 19 M20 13Q14 10 14 15Q14 19 20 17",
        fullscreen: "M3 9V3H9 M15 3H21V9 M21 15V21H15 M9 21H3V15",
        more: "M5 12H5.1 M12 12H12.1 M19 12H19.1",
        down: "M5 9L12 16L19 9",
        plus: "M12 5V19 M5 12H19",
        check: "M4 12L9 17L20 6",
        loop: "M5 8H18L15 5 M18 8L15 11 M19 16H6L9 13 M6 16L9 19",
        camera: "M3 7H7L9 4H15L17 7H21V20H3Z M12 9A4 4 0 1 0 12 17A4 4 0 1 0 12 9",
        screen: "M3 4H21V18H3Z M8 22H16 M12 18V22",
        info: "M12 3A9 9 0 1 0 12 21A9 9 0 1 0 12 3 M12 11V17 M12 7V7.1"
    })
    Shape {
        width: 24; height: 24
        scale: Math.min(icon.width, icon.height) / 24
        transformOrigin: Item.TopLeft
        ShapePath {
            strokeWidth: icon.name === "more" ? 3.8 : 1.7
            strokeColor: icon.tint; fillColor: "transparent"
            capStyle: ShapePath.RoundCap; joinStyle: ShapePath.RoundJoin
            PathSvg { path: icon.paths[icon.name] || icon.paths.info }
        }
    }
}
