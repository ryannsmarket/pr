import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.UserScores 1.0

import "internal"

QmlDialog {
    id: root

    width: 756
    height: 386

    modal: true

    title: qsTrc("userscores", "Export")

    Rectangle {
        id: content

        anchors.fill: parent

        color: ui.theme.popupBackgroundColor

        ExportDialogModel {
            id: exportModel
        }

        QtObject {
            id: privateProperties

            readonly property int sideMargin: 24
        }

        Component.onCompleted: {
            exportModel.load();
            exportModel.selectCurrentNotation()
        }

        RowLayout {
            anchors.fill: parent
            anchors.margins: privateProperties.sideMargin
            spacing: 2 * privateProperties.sideMargin

            ColumnLayout {
                Layout.fillWidth: true
                Layout.preferredWidth: parent.width / 2 - parent.spacing / 2
                spacing: 18

                StyledTextLabel {
                    text: qsTrc("userscores", "Select parts to export")
                    font: ui.theme.bodyBoldFont
                }

                ExportScoresListView {
                    id: exportScoresListView
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    scoresModel: exportModel
                }

                RowLayout {
                    Layout.topMargin: 10
                    spacing: 12

                    FlatButton {
                        Layout.fillWidth: true

                        text: qsTrc("userscores", "Select all")

                        onClicked: {
                            exportModel.setAllSelected(true)
                        }
                    }

                    FlatButton {
                        Layout.fillWidth: true

                        text: qsTrc("userscores", "Clear selection")

                        onClicked: {
                            exportModel.setAllSelected(false)
                        }
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.preferredWidth: parent.width / 2 - parent.spacing / 2
                spacing: 18

                StyledTextLabel {
                    text: qsTrc("userscores", "Export settings")
                    font: ui.theme.bodyBoldFont
                }

                Column {
                    Layout.fillHeight: true
                    Layout.fillWidth: true

                    ExportOptionsView {
                        id: exportOptionsView
                        width: parent.width

                        exportModel: exportModel
                    }
                }

                RowLayout {
                    Layout.alignment: Qt.AlignRight
                    Layout.topMargin: 10
                    spacing: 12

                    FlatButton {
                        text: qsTrc("global", "Cancel")
                        accentButton: !exportButton.enabled

                        onClicked: {
                            root.hide()
                        }
                    }

                    FlatButton {
                        id: exportButton

                        text: qsTrc("userscores", "Export…")
                        enabled: exportModel.selectionLength > 0;
                        accentButton: enabled
                        onClicked: {
                            if (exportModel.exportScores()) {
                                root.hide();
                            }
                        }
                    }
                }
            }
        }
    }
}
