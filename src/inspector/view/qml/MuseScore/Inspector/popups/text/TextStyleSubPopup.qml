/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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
import QtQuick.Layouts 1.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0

import MuseScore.Inspector 1.0

import "../../common"

StyledPopupView {
    id: root

    required property QtObject textSettingsModel

    height: contentHeight

    Column {
        width: parent.width
        spacing: 12

        DropdownPropertyView {
            id: textStyleSection
            titleText: qsTrc("inspector", "Text style")
            propertyItem: root.textSettingsModel ? root.textSettingsModel.textType : null

            visible: !root.textSettingsModel.isDynamicSpecificSettings
            height: visible ? implicitHeight : 0

            model: root.textSettingsModel ? root.textSettingsModel.textStyles : []
        }

        PlacementSection {
            id: textPlacementSection
            propertyItem: root.textSettingsModel ? root.textSettingsModel.textPlacement : null

            visible: !root.textSettingsModel.isDynamicSpecificSettings
        }
    }
}
