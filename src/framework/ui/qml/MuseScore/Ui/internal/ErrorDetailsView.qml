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
import QtQuick.Layouts 1.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

Column {
    id: root

    property string detailedText: ""

    spacing: 16

    ErrorDetailsModel {
        id: detailsModel
    }

    Component.onCompleted: {
        detailsModel.load(root.detailedText)
    }

    Rectangle {
        height: 120
        width: parent.width

        radius: 3

        border.width: 1
        border.color: ui.theme.strokeColor

        visible: detailsView.count > 0

        color: "transparent"

        StyledListView {
            id: detailsView

            anchors.fill: parent
            anchors.margins: 1

            spacing: 0

            model: detailsModel.details

            delegate: Rectangle {
                height: 30
                width: parent.width

                color: model.index % 2 === 0 ? ui.theme.backgroundSecondaryColor : "transparent"

                StyledTextLabel {
                    anchors.fill: parent
                    anchors.leftMargin: 30

                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignLeft

                    width: parent.width

                    textFormat: Qt.RichText
                    text: modelData
                }
            }
        }
    }

    RowLayout {
        spacing: 0

        FlatButton {
            text: qsTrc("global", "Copy")

            onClicked: {
                detailsCopiedMessage.visible = detailsModel.copyDetailsToClipboard()
            }
        }

        StyledIconLabel {
            Layout.leftMargin: 12
            Layout.alignment: Qt.AlignVCenter

            iconCode: IconCode.TICK_RIGHT_ANGLE

            visible: detailsCopiedMessage.visible
        }

        StyledTextLabel {
            id: detailsCopiedMessage

            Layout.leftMargin: 6
            Layout.fillWidth: true
            Layout.maximumWidth: 308
            Layout.alignment: Qt.AlignVCenter

            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignLeft

            wrapMode: Text.WordWrap
            maximumLineCount: 2

            visible: false

            text: qsTrc("global", "Error details have been copied to the clipboard.")
        }
    }
}
