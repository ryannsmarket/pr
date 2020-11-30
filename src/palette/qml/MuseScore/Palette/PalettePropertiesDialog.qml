import QtQuick 2.9

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Palette 1.0

QmlDialog {
    id: root

    width: 280
    height: 370

    title: qsTrc("palette", "Palette Properties")

    property var properties

    Rectangle {
        color: ui.theme.backgroundPrimaryColor

        PalettePropertiesModel {
            id: propertiesModel
        }

        Component.onCompleted: {
            propertiesModel.load(properties)
        }

        Column {
            anchors.fill: parent

            readonly property int margins: 12
            anchors.margins: margins

            spacing: 12

            StyledTextLabel {
                text: qsTrc("palette", "Name")
                font: ui.theme.bodyBoldFont
            }

            TextInputField {
                currentText: propertiesModel.name

                onCurrentTextEdited: {
                    propertiesModel.name = newTextValue
                }
            }

            SeparatorLine { anchors.margins: -parent.margins }

            StyledTextLabel {
                text: qsTrc("palette", "Cell size")
                font: ui.theme.bodyBoldFont
            }

            Grid {
                width: parent.width

                columns: 2
                spacing: 16

                Repeater {
                    id: repeater

                    model: [
                        { title: qsTrc("palette", "Width"), value: propertiesModel.cellWidth, incrementStep: 1 },
                        { title: qsTrc("palette", "Height"), value: propertiesModel.cellHeight, incrementStep: 1 },
                        { title: qsTrc("palette", "Element offset"), value: propertiesModel.elementOffset, measureUnit: qsTrc("palette", "sp"), incrementStep: 0.1 },
                        { title: qsTrc("palette", "Scale"), value: propertiesModel.scaleFactor, incrementStep: 0.1 }
                    ]

                    function setValue(index, value) {
                        if (index === 0) {
                            propertiesModel.cellWidth = value
                        } else if (index === 1) {
                            propertiesModel.cellHeight = value
                        } else if (index === 2) {
                            propertiesModel.elementOffset = value
                        } else if (index === 3) {
                            propertiesModel.scaleFactor = value
                        }
                    }

                    Column {
                        width: parent.width / 2 - 8

                        spacing: 8

                        StyledTextLabel {
                            text: modelData["title"]
                        }

                        IncrementalPropertyControl {
                            currentValue: modelData["value"]
                            measureUnitsSymbol: Boolean(modelData["measureUnit"]) ? modelData["measureUnit"] : ""
                            step: modelData["incrementStep"]

                            onValueEdited: {
                                repeater.setValue(model.index, newValue)
                            }
                        }
                    }
                }
            }

            CheckBox {
                text: qsTrc("palette", "Show grid")

                checked: propertiesModel.showGrid

                onClicked: {
                    propertiesModel.showGrid = !checked
                }
            }

            Item { height: 1; width: parent.width }

            Row {
                width: parent.width
                height: childrenRect.height + 20

                spacing: 4

                FlatButton {
                    text: qsTrc("global", "Cancel")

                    width: parent.width / 2

                    onClicked: {
                        propertiesModel.reject()
                        root.hide()
                    }
                }

                FlatButton {
                    text: qsTrc("global", "Ok")

                    width: parent.width / 2

                    onClicked: {
                        root.hide()
                    }
                }
            }
        }
    }
}
