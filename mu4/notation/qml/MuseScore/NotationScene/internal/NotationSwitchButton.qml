import QtQuick 2.7
import QtQuick.Layouts 1.3

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

FlatRadioButton {
    id: root

    property string title: ""

    signal closeRequested()

    normalStateColor: ui.theme.backgroundSecondaryColor
    hoverStateColor: selectedStateColor
    pressedStateColor: selectedStateColor
    selectedStateColor: ui.theme.backgroundPrimaryColor

    width: 110
    radius: 0

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 12

        StyledTextLabel {
            Layout.alignment: Qt.AlignLeft
            Layout.fillHeight: true
            Layout.fillWidth: true

            horizontalAlignment: Text.AlignLeft

            text: root.title
            font.pixelSize: 12
        }

        FlatButton {
            Layout.preferredHeight: 14
            Layout.preferredWidth: height
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter

            normalStateColor: "transparent"
            icon: IconCode.CLOSE_X_ROUNDED
            iconPixelSize: 12
            onClicked: root.closeRequested()
        }

        SeparatorLine { orientation: Qt.Vertical }
    }
}
