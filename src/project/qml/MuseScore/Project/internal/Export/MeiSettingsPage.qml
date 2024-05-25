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

import Muse.UiComponents 1.0

ExportSettingsPage {
    id: root

    CheckBox {
        width: parent.width
        text: qsTrc("project/export", "Include page and system breaks")

        navigation.name: "MeiExportLayout"
        navigation.panel: root.navigationPanel
        navigation.row: root.navigationOrder + 1

        checked: root.model.meiExportLayout
        onClicked: {
            root.model.meiExportLayout = !checked
        }
    }

    CheckBox {
        width: parent.width
        text: qsTrc("project/export", "Use MuseScore Ids")

        navigation.name: "MeiUseMscoreIds"
        navigation.panel: root.navigationPanel
        navigation.row: root.navigationOrder + 1

        checked: root.model.meiUseMscoreIds
        onClicked: {
            root.model.meiUseMscoreIds = !checked
        }
    }
}
