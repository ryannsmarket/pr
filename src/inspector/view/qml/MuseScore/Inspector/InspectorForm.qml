/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Inspector 1.0

import "."

Rectangle {
    id: root

    property alias model: flickableArea.model
    property alias notationView: popupController.notationView

    property NavigationSection navigationSection: null
    property NavigationPanel navigationPanel: flickableArea.count > 0 ? flickableArea.itemAt(0).navigationPanel : null // first panel
    property int navigationOrderStart: 0

    color: ui.theme.backgroundPrimaryColor

    onVisibleChanged: {
        inspectorListModel.setInspectorVisible(root.visible)
    }

    function focusFirstItem() {
        var item = flickableArea.itemAt(0)
        if (item) {
            item.navigation.requestActive()
        }
    }

    QtObject {
        id: prv

        function closePreviousOpenedPopup(newOpenedPopup, visualControl) {
            if (Boolean(popupController.popup) && popupController.popup !== newOpenedPopup) {
                popupController.popup.close()
            }

            popupController.visualControl = visualControl
            popupController.popup = newOpenedPopup
        }
    }

    InspectorPopupController {
        id: popupController
    }

    StyledListView {
        id: flickableArea
        anchors.fill: parent
        anchors.margins: 12

        function ensureContentVisible(invisibleContentHeight) {
            if (flickableArea.contentY + invisibleContentHeight > 0) {
                flickableArea.contentY += invisibleContentHeight
            } else {
                flickableArea.contentY = 0
            }
        }

        flickableDirection: Flickable.VerticalFlick

        Behavior on contentY {
            NumberAnimation { duration: 250 }
        }

        ScrollBar.vertical: StyledScrollBar {}

        model: InspectorListModel {
            id: inspectorListModel
        }

        delegate: Column {
            width: flickableArea.width

            spacing: 12

            property var navigationPanel: _item.navigationPanel

            SeparatorLine {
                anchors.margins: -12

                visible: model.index !== 0
            }

            InspectorSectionDelegate {
                id: _item

                sectionModel: model.inspectorSectionModel
                anchorItem: root
                navigationPanel.section: root.navigationSection
                navigationPanel.order: root.navigationOrderStart + model.index
                navigationPanel.onOrderChanged: {
                    if (model.index === 0) {
                        root.navigationOrderStart = navigationPanel.order
                    }
                }

                onReturnToBoundsRequested: {
                    flickableArea.returnToBounds()
                }

                onEnsureContentVisibleRequested: function(invisibleContentHeight) {
                    flickableArea.ensureContentVisible(invisibleContentHeight)
                }

                onPopupOpened: function(openedPopup, visualControl) {
                    prv.closePreviousOpenedPopup(openedPopup, visualControl)
                }
            }
        }
    }
}
