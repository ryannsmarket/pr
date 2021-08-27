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
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Diagnostics 1.0

Rectangle {

    id: root

    objectName: "DiagnosticEngravingElementsPanel"

    color: ui.theme.backgroundPrimaryColor

    Component.onCompleted: {
        elementsModel.init()
        elementsModel.reload()
    }

    EngravingElementsModel {
        id: elementsModel
    }

    Item {
        id: tools
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 8
        height: childrenRect.height

        FlatButton {
            id: reloadBtn
            text: "Reload"
            onClicked: elementsModel.reload()
        }

        StyledTextLabel {
            id: summaryLabel
            anchors.top: parent.top
            anchors.left: reloadBtn.right
            anchors.right: parent.right
            anchors.leftMargin: 8
            height: 32
            verticalAlignment: Text.AlignTop
            horizontalAlignment: Text.AlignLeft
            text: elementsModel.summary
            visible: true
        }

        FlatButton {
            id: moreBtn
            anchors.right: parent.right
            text: infoLable.visible ? "Less" : "More"
            onClicked: infoLable.visible = !infoLable.visible
        }

        StyledTextLabel {
            id: infoLable
            anchors.top: reloadBtn.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            height: visible ? implicitHeight : 0
            verticalAlignment: Text.AlignTop
            horizontalAlignment: Text.AlignLeft
            text: elementsModel.info
            visible: false
        }
    }

    TreeView {
        id: view
        anchors.top: tools.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        clip: true

        headerVisible: false

        model: elementsModel

        function positionViewAtIndex(index) {
            var rows = -1
            while (index.valid) {
                var r = index.row + 1
                rows += r
                index = view.model.parent(index)
            }

            __listView.positionViewAtIndex(rows, ListView.Center)
        }

        function expandBranch(index) {
            var idxs = []
            var parent = view.model.parent(index)
            while (parent.valid) {
                idxs.push(parent)
                parent = view.model.parent(parent)
            }

            for(var i = (idxs.length - 1); i >= 0; --i) {
                var idx = idxs[i]
                view.expand(idx)
            }
        }

        function collapseBranch(index) {
            var idxs = []
            idxs.push(index)
            var parent = view.model.parent(index)
            while (parent.valid) {
                idxs.push(parent)
                parent = view.model.parent(parent)
            }

            for(var i = 0; i < idxs.length; ++i) {
                var idx = idxs[i]
                view.collapse(idx)
            }
        }

        TableViewColumn {
            role: "itemData"
        }

        style: TreeViewStyle {
            indentation: styleData.depth
            rowDelegate: Rectangle {
                height: 24
                width: parent.width
                color: styleData.row%2 == 0 ? root.color : ui.theme.backgroundSecondaryColor
            }
        }

        itemDelegate: Item {
            id: item

            StyledTextLabel {
                id: secLabel
                anchors.fill: parent
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignLeft
                elide: Text.ElideNone
                text: styleData.value.name
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (!styleData.isExpanded) {
                        view.expand(styleData.index)
                    } else {
                        view.collapse(styleData.index)
                    }
                }
            }

            FlatButton {
                id: selBtn
                anchors.right: parent.right
                anchors.rightMargin: 8
                height: 16
                width: 32
                anchors.verticalCenter: parent.verticalCenter
                text: styleData.value.selected ? "Unsel" : "Sel"
                onClicked: elementsModel.select(styleData.index, !styleData.value.selected)
            }
        }
    }
}
