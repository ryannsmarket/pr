import QtQuick 2.9
import MuseScore.Inspectors 3.3
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import "../../common"

PopupViewButton {
    id: root

    property alias model: horizontalFramePopup.model

    icon: IconCode.HORIZONTAL_FRAME
    text: qsTr("Horizontal frames")

    visible: root.model ? !root.model.isEmpty : false

    HorizontalFramePopup {
        id: horizontalFramePopup

        x: popupPositionX
        y: popupPositionY
        arrowX: parent.x + parent.width / 2
        width: popupAvailableWidth
    }
}
