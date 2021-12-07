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
import QtQuick.Layouts 1.15

import MuseScore.UiComponents 1.0
import MuseScore.Preferences 1.0

import "internal"

PreferencesPage {
    id: root

    Component.onCompleted: {
        preferencesModel.load()
    }

    CanvasPreferencesModel {
        id: preferencesModel
    }

    Column {
        anchors.fill: parent
        spacing: root.sectionsSpacing

        ZoomSection {
            defaultZoom: preferencesModel.defaultZoom
            zoomTypes: preferencesModel.zoomTypes()
            mouseZoomPrecision: preferencesModel.mouseZoomPrecision

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 1

            onDefaultZoomTypeChangeRequested: {
                preferencesModel.setDefaultZoomType(zoomType)
            }

            onDefaultZoomLevelChangeRequested: {
                preferencesModel.setDefaultZoomLevel(zoomLevel)
            }

            onMouseZoomPrecisionChangeRequested: {
                preferencesModel.mouseZoomPrecision = zoomPrecision
            }
        }

        SeparatorLine { }

        ScrollPagesSection {
            orientation: preferencesModel.scrollPagesOrientation
            limitScrollArea: preferencesModel.limitScrollArea

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 2

            onOrientationChangeRequested: {
                preferencesModel.scrollPagesOrientation = orientation
            }

            onLimitScrollAreaChangeRequested: {
                preferencesModel.limitScrollArea = limit
            }
        }

        SeparatorLine { }

        MiscellaneousSection {
            selectionProximity: preferencesModel.selectionProximity

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 3

            onSelectionProximityChangeRequested: {
                preferencesModel.selectionProximity = proximity
            }
        }
    }
}
