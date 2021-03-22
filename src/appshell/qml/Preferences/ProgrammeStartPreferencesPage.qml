import QtQuick 2.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Preferences 1.0

Item {
    ProgrammeStartPreferencesModel {
        id: programmeStartModel
    }

    Column {
        anchors.fill: parent

        spacing: 20

        StyledTextLabel {
            text: qsTrc("appshell", "Programme Start")
            font: ui.theme.bodyBoldFont
        }

        RadioButtonGroup {
            spacing: 16
            orientation: Qt.Vertical

            width: parent.width

            model: programmeStartModel.startupModes

            delegate: Row {
                spacing: 0

                RoundedRadioButton {
                    anchors.verticalCenter: parent.verticalCenter

                    width: 220
                    padding: 0
                    spacing: 6

                    checked: modelData.checked

                    onClicked: {
                        programmeStartModel.setCurrentStartupMode(model.index)
                    }

                    StyledTextLabel {
                        text: modelData.title
                        horizontalAlignment: Qt.AlignLeft
                    }
                }

                FilePicker {
                    width: 240

                    dialogTitle: qsTrc("appshell", "Choose Starting Score")
                    filter: programmeStartModel.scorePathFilter()

                    visible: modelData.canSelectScorePath
                    path: modelData.scorePath

                    onPathEdited: {
                        programmeStartModel.setStartupScorePath(newPath)
                    }
                }
            }
        }

        ListView {
            spacing: 16
            interactive: false

            width: parent.width
            height: contentHeight

            model: programmeStartModel.panels

            delegate: CheckBox {
                text: modelData.title
                checked: modelData.visible

                onClicked: {
                    programmeStartModel.setPanelVisible(model.index, !checked)
                }
            }
        }
    }
}
