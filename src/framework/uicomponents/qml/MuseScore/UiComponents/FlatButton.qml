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
import MuseScore.UiComponents 1.0

FocusableControl {
    id: root

    property alias icon: buttonIcon.iconCode
    property alias text: textLabel.text
    property string hint

    property alias iconFont: buttonIcon.font
    property alias textFont: textLabel.font

    property color normalStateColor: privateProperties.defaultColor
    property color hoveredStateColor: privateProperties.defaultColor
    property color pressedStateColor: privateProperties.defaultColor
    property bool accentButton: false

    property int orientation: Qt.Vertical

    property bool isClickOnKeyNavTriggered: true

    signal clicked()
    signal pressAndHold()

    QtObject {
        id: privateProperties

        property color defaultColor: accentButton ? ui.theme.accentColor : ui.theme.buttonColor
        property bool isVertical: orientation === Qt.Vertical
    }

    height: contentWrapper.height + 14
    width: (Boolean(text) ? Math.max(contentWrapper.width + 32, privateProperties.isVertical ? 132 : 0) : contentWrapper.width + 16)

    opacity: root.enabled ? 1.0 : ui.theme.itemOpacityDisabled

    mouseArea.onClicked: root.clicked()
    mouseArea.onPressAndHold: root.pressAndHold()

    mouseArea.hoverEnabled: true
    mouseArea.onContainsMouseChanged: {
        if (!Boolean(root.hint)) {
            return
        }

        if (mouseArea.containsMouse) {
            ui.tooltip.show(this, root.hint)
        } else {
            ui.tooltip.hide(this)
        }
    }

    navigation.onTriggered: {
        if (root.isClickOnKeyNavTriggered) {
            root.clicked()
        }
    }

    background.color: normalStateColor
    background.opacity: ui.theme.buttonOpacityNormal
    background.radius: 3

    Item {
        id: contentWrapper

        property int spacing: Boolean(!buttonIcon.isEmpty) && Boolean(textLabel.text) ? 4 : 0

        anchors.verticalCenter: parent ? parent.verticalCenter : undefined
        anchors.horizontalCenter: parent ? parent.horizontalCenter : undefined

        height: !privateProperties.isVertical ? Math.max(buttonIcon.height, textLabel.height) : buttonIcon.height + textLabel.height + spacing
        width: privateProperties.isVertical ? Math.max(textLabel.width, buttonIcon.width) : buttonIcon.width + textLabel.width + spacing

        StyledIconLabel {
            id: buttonIcon

            font.pixelSize: isEmpty ? 0 : ui.theme.iconsFont.pixelSize
        }

        StyledTextLabel {
            id: textLabel

            height: text === "" ? 0 : implicitHeight
            horizontalAlignment: Text.AlignHCenter
        }

        states: [
            State {
                when: !privateProperties.isVertical
                AnchorChanges {
                    target: buttonIcon
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                }
                AnchorChanges {
                    target: textLabel
                    anchors.left: buttonIcon.right
                    anchors.verticalCenter: parent.verticalCenter
                }
                PropertyChanges {
                    target: textLabel
                    anchors.leftMargin: contentWrapper.spacing
                }
            },
            State {
                when: privateProperties.isVertical
                AnchorChanges {
                    target: buttonIcon
                    anchors.top: parent.top
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                AnchorChanges {
                    target: textLabel
                    anchors.top: buttonIcon.bottom
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                PropertyChanges {
                    target: textLabel
                    anchors.leftMargin: contentWrapper.spacing
                }
            }
        ]
    }

    states: [
        State {
            name: "PRESSED"
            when: mouseArea.pressed

            PropertyChanges {
                target: root.background
                color: pressedStateColor
                opacity: ui.theme.buttonOpacityHit
            }
        },

        State {
            name: "HOVERED"
            when: mouseArea.containsMouse && !mouseArea.pressed

            PropertyChanges {
                target: root.background
                color: hoveredStateColor
                opacity: ui.theme.buttonOpacityHover
            }
        }
    ]
}
