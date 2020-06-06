import QtQuick 2.9
import QtQuick.Layouts 1.3
import MuseScore.Inspectors 3.3
import "../../common"

PopupViewButton {
    id: root

    property alias model: keySignaturePopup.model

    icon: IconNameTypes.KEY_SIGNATURE
    text: qsTr("Key signatures")

    visible: root.model ? !root.model.isEmpty : false

    KeySignaturePopup {
        id: keySignaturePopup

        x: popupPositionX
        y: popupPositionY
        arrowX: parent.x + parent.width / 2
        width: popupAvailableWidth
    }
}
