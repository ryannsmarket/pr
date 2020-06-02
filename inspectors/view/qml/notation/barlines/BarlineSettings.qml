import QtQuick 2.9
import QtQuick.Layouts 1.3
import MuseScore.Inspectors 3.3
import "../../common"

PopupViewButton {
    id: root

    property alias barlineSettingsModel: barlinePopup.barlineSettingsModel
    property alias staffSettingsModel: barlinePopup.staffSettingsModel

    icon: IconNameTypes.SECTION_BREAK
    text: qsTr("Barlines")

    visible: root.barlineSettingsModel ? !root.barlineSettingsModel.isEmpty : false

    BarlinePopup {
        id: barlinePopup

        x: popupPositionX
        y: popupPositionY
        arrowX: parent.x + parent.width / 2
        width: popupAvailableWidth
    }
}
