import QtQuick 2.9
import MuseScore.Inspectors 3.3
import "../../common"

PopupViewButton {
    id: root

    property alias model: textFramePopup.model

    icon: IconNameTypes.TEXT_FRAME
    text: qsTr("Text frame")

    visible: root.model ? !root.model.isEmpty : false

    TextFramePopup {
        id: textFramePopup

        x: popupPositionX
        y: popupPositionY
        arrowX: parent.x + parent.width / 2
        width: popupAvailableWidth
    }
}
