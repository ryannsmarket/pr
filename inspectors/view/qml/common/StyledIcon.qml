import QtQuick 2.1
import QtGraphicalEffects 1.0

Item {
    id: root

    property alias icon: image.source
    property alias sourceSize: image.sourceSize
    property alias color: colorOverlay.color

    implicitHeight: 16
    implicitWidth: 16

    Image {
        id: image

        anchors.centerIn: parent

        height: 16
        width: implicitWidth

        fillMode: Image.PreserveAspectFit
    }

    ColorOverlay {
        id: colorOverlay

        anchors.fill: image
        source: image
        color: globalStyle.buttonText
    }
}
