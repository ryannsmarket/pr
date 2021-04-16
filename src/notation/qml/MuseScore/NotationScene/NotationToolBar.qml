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
import MuseScore.Ui 1.0
import MuseScore.NotationScene 1.0
import MuseScore.UiComponents 1.0

Rectangle {
    id: root

    property alias keynav: keynavSub

    signal activeFocusRequested()

    Component.onCompleted: {
        toolbarModel.load()
    }

    KeyNavigationSubSection {
        id: keynavSub
        name: "NotationToolBar"
        onActiveChanged: {
            if (active) {
                root.activeFocusRequested()
                root.forceActiveFocus()
            }
        }
    }

    NotationToolBarModel {
        id: toolbarModel
    }

    ListView {
        id: view

        anchors.verticalCenter: parent.verticalCenter

        width: contentWidth
        height: contentItem.childrenRect.height

        orientation: Qt.Horizontal
        interactive: false
        spacing: 2

        model: toolbarModel

        delegate: FlatButton {
            text: model.title
            icon: model.icon
            iconFont: ui.theme.toolbarIconsFont

            hint: model.hint
            enabled: model.enabled
            textFont: ui.theme.tabFont

            keynav.subsection: keynavSub
            keynav.name: model.title
            keynav.order: model.index
            keynav.enabled: model.enabled

            normalStateColor: "transparent"
            orientation: Qt.Horizontal

            onClicked: {
                toolbarModel.handleAction(model.code)
            }
        }
    }
}
