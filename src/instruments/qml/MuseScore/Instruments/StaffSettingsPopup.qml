import QtQuick 2.9

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Instruments 1.0

StyledPopup {
    id: root

    height: contentColumn.implicitHeight + topPadding + bottomPadding
    width: parent.width

    function load(staff) {
        settingsModel.load(staff)
    }

    StaffSettingsModel {
        id: settingsModel
    }

    Column {
        id: contentColumn

        width: parent.width

        spacing: 12

        StyledTextLabel {
            text: qsTrc("instruments", "Staff type")
        }

        StyledComboBox {
            width: parent.width

            textRoleName: "text"
            valueRoleName: "value"

            model: {
                var types = settingsModel.allStaffTypes()
                var result = []

                for (var i = 0; i < types.length; ++i) {
                    result.push({ text: types[i].title, value: types[i].value })
                }

                return result
            }

            onValueChanged: {
                settingsModel.setStaffType(value)
            }
        }

        SeparatorLine {
            anchors.leftMargin: -root.leftPadding + root.borderWidth
            anchors.rightMargin: -root.rightPadding + root.borderWidth
        }

        StyledTextLabel {
            text: qsTrc("instruments", "Voices visible in the score")
        }

        ListView {
            height: 20
            width: parent.width

            spacing: 32
            orientation: ListView.Horizontal

            model: settingsModel.voices

            delegate: CheckBox {
                text: modelData.title
                checked: modelData.visible

                onClicked: {
                    settingsModel.setVoiceVisible(model.index, !checked)
                }
            }
        }

        SeparatorLine {
            anchors.leftMargin: -root.leftPadding + root.borderWidth
            anchors.rightMargin: -root.rightPadding + root.borderWidth
        }

        CheckBox {
            text: qsTrc("instruments", "Small staff")

            checked: settingsModel.isSmallStaff

            onClicked: {
                settingsModel.setIsSmallStaff(!checked)
            }
        }

        CheckBox {
            width: parent.width

            text: qsTrc("instruments", "Hide all measures that do not contain notation (cutaway)")
            wrapMode: Text.WordWrap

            checked: settingsModel.cutawayEnabled

            onClicked: {
                settingsModel.setCutawayEnabled(!checked)
            }
        }

        SeparatorLine {
            anchors.leftMargin: -root.leftPadding + root.borderWidth
            anchors.rightMargin: -root.rightPadding + root.borderWidth
        }

        FlatButton {
            width: parent.width

            text: qsTrc("instruments", "Create a linked staff")

            onClicked: {
                settingsModel.createLinkedStaff()
            }
        }

        StyledTextLabel {
            width: parent.width

            text: qsTrc("instruments", "Note: linked staves contain identical information.")
            wrapMode: Text.WordWrap
        }
    }
}
