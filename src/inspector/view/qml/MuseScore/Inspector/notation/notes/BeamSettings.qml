/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
import QtQuick 2.15
import QtQuick.Controls 2.15

import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

import "../../common"
import "internal"

FocusableItem {
    id: root

    property QtObject model: null
    readonly property QtObject beamModesModel: model ? model.beamModesModel : null

    property NavigationPanel navigationPanel: null
    property int navigationRowOffset: 1

    implicitHeight: contentColumn.height
    width: parent.width

    Column {
        id: contentColumn

        width: parent.width
        spacing: 12

        BeamTypeSelector {
            id: beamTypeSection
            titleText: qsTrc("inspector", "Beam types")
            propertyItem: root.beamModesModel ? root.beamModesModel.mode : null
            enabled: root.beamModesModel && !root.beamModesModel.isEmpty

            navigation.panel: root.navigationPanel
            navigationRowStart: root.navigationRowOffset
            navigation.enabled: root.enabled
        }

        Column {
            spacing: 12

            height: implicitHeight
            width: parent.width

            enabled: root.model ? !root.model.isEmpty : false

            SeparatorLine {
                anchors.margins: -10
                visible: featheringControlsColumn.visible
            }

            Column {
                id: featheringControlsColumn

                spacing: 12

                height: implicitHeight
                width: parent.width

                visible: root.beamModesModel ? root.beamModesModel.isFeatheringAvailable.isUndefined || root.beamModesModel.isFeatheringAvailable.value
                                             : false

                StyledTextLabel {
                    id: featheredBeamsLabel
                    text: qsTrc("inspector", "Feathered beams")
                }

                RadioButtonGroup {
                    id: featheredBeamsButtonList

                    property int navigationRowStart: beamTypeSection.navigationRowEnd + 1
                    property int navigationRowEnd: navigationRowOffset + model.length

                    height: 30
                    width: parent.width

                    model: [
                        { text: qsTrc("inspector", "None"), value: Beam.FEATHERING_NONE, title: qsTrc("inspector", "None") },
                        { iconCode: IconCode.FEATHERED_LEFT_HEIGHT, value: Beam.FEATHERING_LEFT, title: qsTrc("inspector", "Left") },
                        { iconCode: IconCode.FEATHERED_RIGHT_HEIGHT, value: Beam.FEATHERING_RIGHT, title: qsTrc("inspector", "Right") }
                    ]

                    delegate: FlatRadioButton {
                        iconCode: modelData["iconCode"] ?? IconCode.NONE
                        text: modelData["text"] ?? ""

                        navigation.name: text
                        navigation.panel: root.navigationPanel
                        navigation.row: featheredBeamsButtonList.navigationRowStart + index
                        navigation.enabled: root.enabled && featheringControlsColumn.visible
                        navigation.accessible.name: featheredBeamsLabel.text + " " + text

                        checked: root.beamModesModel && !(root.model.featheringHeightLeft.isUndefined || root.model.featheringHeightRight.isUndefined)
                                 ? root.model.featheringMode === modelData["value"]
                                 : false
                        onToggled: {
                            root.model.featheringMode = modelData["value"]
                        }
                    }
                }

                Item {
                    height: childrenRect.height
                    width: parent.width

                    visible: root.model && root.model.isFeatheringHeightChangingAllowed

                    SpinBoxPropertyView {
                        id: featheringLeftSection
                        anchors.left: parent.left
                        anchors.right: parent.horizontalCenter
                        anchors.rightMargin: 4

                        titleText: qsTrc("inspector", "Feathering left")
                        propertyItem: root.model ? root.model.featheringHeightLeft : null
                        enabled: root.beamModesModel ? root.beamModesModel.isFeatheringAvailable : false

                        icon: IconCode.FEATHERED_LEFT_HEIGHT
                        maxValue: 4
                        minValue: 0
                        step: 0.1

                        navigation.name: "FeatheringLeft"
                        navigation.panel: root.navigationPanel
                        navigation.row: featheredBeamsButtonList.navigationRowEnd + 1
                    }

                    SpinBoxPropertyView {
                        id: featheringRightSection
                        anchors.left: parent.horizontalCenter
                        anchors.leftMargin: 4
                        anchors.right: parent.right

                        titleText: qsTrc("inspector", "Feathering right")
                        propertyItem: root.model ? root.model.featheringHeightRight : null
                        enabled: root.beamModesModel ? root.beamModesModel.isFeatheringAvailable : false

                        icon: IconCode.FEATHERED_RIGHT_HEIGHT
                        iconMode: IncrementalPropertyControl.Right
                        maxValue: 4
                        minValue: 0
                        step: 0.1

                        navigation.name: "FeatheringRight"
                        navigation.panel: root.navigationPanel
                        navigation.row: featheringLeftSection.navigationRowEnd + 1
                    }
                }
            }

            SeparatorLine {
                anchors.margins: -10
                visible: featheringControlsColumn.visible
            }

            FlatButton {
                id: forceHorizontalButton
                width: parent.width

                text: qsTrc("inspector", "Force horizontal")

                navigation.name: "ForceHorizontal"
                navigation.panel: root.navigationPanel
                navigation.row: featheringRightSection.navigationRowEnd + 1

                onClicked: {
                    if (!root.model) {
                        return
                    }

                    root.model.forceHorizontal()
                }
            }

            ExpandableBlank {
                id: showItem
                isExpanded: false

                title: isExpanded ? qsTrc("inspector", "Show less") : qsTrc("inspector", "Show more")

                width: parent.width

                navigation.panel: root.navigationPanel
                navigation.row: forceHorizontalButton.navigation.row + 1

                contentItemComponent: Column {
                    height: implicitHeight
                    width: root.width

                    spacing: 12

                    InspectorPropertyView {
                        id: beamHeight
                        titleText: qsTrc("inspector", "Beam height")
                        propertyItem: root.model ? root.model.beamVectorX : null

                        Item {
                            height: childrenRect.height
                            width: parent.width

                            IncrementalPropertyControl {
                                id: beamHightRightControl

                                anchors.left: parent.left
                                anchors.right: lockButton.left
                                anchors.rightMargin: 6

                                icon: IconCode.BEAM_RIGHT_Y_POSITION
                                isIndeterminate: root.model ? root.model.beamVectorX.isUndefined : false
                                currentValue: root.model ? root.model.beamVectorX.value : 0

                                navigation.name: "BeamHightRightControl"
                                navigation.panel: root.navigationPanel
                                navigation.row: showItem.navigation.row + 1
                                navigation.accessible.name: beamHeight.titleText + " " + qsTrc("inspector", "Right") + " " + currentValue
                                navigation.enabled: root.enabled && beamHeight.enabled

                                onValueEdited: { root.model.beamVectorX.value = newValue }
                            }

                            FlatToggleButton {
                                id: lockButton

                                anchors.horizontalCenter: parent.horizontalCenter
                                anchors.verticalCenter: beamHightRightControl.verticalCenter

                                height: 20
                                width: 20

                                icon: checked ? IconCode.LOCK_CLOSED : IconCode.LOCK_OPEN

                                checked: root.model ? root.model.isBeamHeightLocked : false

                                navigation.panel: root.navigationPanel
                                navigation.name: "FontStyle" + model.index
                                navigation.row: showItem.navigation.row + 2
                                navigation.accessible.name: qsTrc("inspector", "Lock")
                                navigation.enabled: root.enabled && beamHeight.enabled

                                onToggled: {
                                    root.model.isBeamHeightLocked = !root.model.isBeamHeightLocked
                                }
                            }

                            IncrementalPropertyControl {
                                anchors.left: lockButton.right
                                anchors.leftMargin: 6
                                anchors.right: parent.right

                                icon: IconCode.BEAM_LEFT_Y_POSITION
                                iconMode: IncrementalPropertyControl.Right
                                isIndeterminate: root.model ? root.model.beamVectorY.isUndefined : false
                                currentValue: root.model ? root.model.beamVectorY.value : 0

                                navigation.name: "BeamHightLeftControl"
                                navigation.panel: root.navigationPanel
                                navigation.row: showItem.navigation.row + 3
                                navigation.accessible.name: beamHeight.titleText + " " + qsTrc("inspector", "Left") + " " + currentValue
                                navigation.enabled: root.enabled && beamHeight.enabled

                                onValueEdited: { root.model.beamVectorY.value = newValue }
                            }
                        }
                    }
                }
            }
        }
    }
}
