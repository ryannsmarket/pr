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

import MuseScore.AppShell 1.0
import MuseScore.UiComponents 1.0
import MuseScore.NotationScene 1.0

Rectangle {
    id: root

    NotationStatusBarModel {
        id: model
    }

    Component.onCompleted: {
        model.load()
    }

    MouseArea {
        anchors.fill: parent

        onClicked: root.forceActiveFocus()
    }

    StyledTextLabel {
        anchors.left: parent.left
        anchors.leftMargin: 12
        anchors.right: statusBarRow.left
        anchors.verticalCenter: parent.verticalCenter

        horizontalAlignment: Text.AlignLeft

        text: model.accessibilityInfo
    }

    Row {
        id: statusBarRow

        anchors.right: parent.right
        anchors.rightMargin: hiddenControlsMenuButton.visible ? 4 : 12

        height: parent.height

        spacing: 10

        SeparatorLine { orientation: Qt.Vertical; visible: workspaceControl.visible }

        FlatButton {
            id: workspaceControl

            anchors.verticalCenter: parent.verticalCenter

            text: model.currentWorkspaceAction.title
            normalStateColor: "transparent"

            onClicked: {
                Qt.callLater(model.selectWorkspace)
            }
        }

        SeparatorLine { orientation: Qt.Vertical; visible: concertPitchControl.visible }

        ConcertPitchControl {
            id: concertPitchControl

            anchors.verticalCenter: parent.verticalCenter

            text: model.concertPitchAction.title
            icon: model.concertPitchAction.icon
            checked: model.concertPitchAction.checked
            enabled: model.concertPitchAction.enabled

            onToggleConcertPitchRequested: {
                model.toggleConcertPitch()
            }
        }

        SeparatorLine { orientation: Qt.Vertical }

        ViewModeControl {
            anchors.verticalCenter: parent.verticalCenter

            currentViewMode: model.currentViewMode
            availableViewModeList: model.availableViewModeList

            onChangeCurrentViewModeRequested: {
                model.setCurrentViewMode(newViewMode)
            }
        }

        ZoomControl {
            anchors.verticalCenter: parent.verticalCenter

            enabled: model.zoomEnabled
            currentZoomPercentage: model.currentZoomPercentage
            minZoomPercentage: model.minZoomPercentage()
            maxZoomPercentage: model.maxZoomPercentage()
            availableZoomList: model.availableZoomList

            onChangeZoomPercentageRequested: {
                model.currentZoomPercentage = newZoomPercentage
            }

            onChangeZoomRequested: {
                model.setCurrentZoomIndex(newZoomIndex)
            }

            onZoomInRequested: {
                model.zoomIn()
            }

            onZoomOutRequested: {
                model.zoomOut()
            }
        }

        SeparatorLine { orientation: Qt.Vertical; visible: hiddenControlsMenuButton.visible }

        MenuButton {
            id: hiddenControlsMenuButton

            anchors.verticalCenter: parent.verticalCenter

            visible: !concertPitchControl.visible ||
                     !workspaceControl.visible

            menuModel: {
                var result = []

                if (!concertPitchControl.visible) {
                    result.push(model.concertPitchAction)
                }

                if (!workspaceControl.visible) {
                    result.push(model.currentWorkspaceAction)
                }

                return result
            }

            onHandleAction: {
                model.handleAction(actionCode)
            }
        }
    }
}
