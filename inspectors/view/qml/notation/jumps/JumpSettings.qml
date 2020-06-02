import QtQuick 2.9
import QtQuick.Layouts 1.3
import MuseScore.Inspectors 3.3
import "../../common"

PopupViewButton {
    id: root

    property alias model: jumpPopup.model

    icon: IconNameTypes.JUMP
    text: qsTr("Jumps")

    visible: root.model ? !root.model.isEmpty : false

    JumpPopup {
        id: jumpPopup

        x: popupPositionX
        y: popupPositionY
        arrowX: parent.x + parent.width / 2
        width: popupAvailableWidth
    }
}
