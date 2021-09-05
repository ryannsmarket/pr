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
import MuseScore.UiComponents 1.0
import MuseScore.Preferences 1.0

import "internal"

PreferencesPage {
    id: root

    contentHeight: content.height

    AppearancePreferencesModel {
        id: appearanceModel
    }

    Component.onCompleted: {
        appearanceModel.init()
    }

    Column {
        id: content

        width: parent.width
        spacing: 24

        ThemesSection {
            width: content.width

            themes: appearanceModel.highContrastEnabled ? appearanceModel.highContrastThemes : appearanceModel.generalThemes
            currentThemeCode: appearanceModel.currentThemeCode
            highContrastEnabled: appearanceModel.highContrastEnabled
            accentColors: appearanceModel.accentColors
            currentAccentColorIndex: appearanceModel.currentAccentColorIndex

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 1

            onThemeChangeRequested: {
                appearanceModel.currentThemeCode = newThemeCode
            }

            onHighContrastChangeRequested: {
                appearanceModel.highContrastEnabled = enabled
            }

            onAccentColorChangeRequested: {
                appearanceModel.currentAccentColorIndex = newColorIndex
            }

            onFocusChanged: {
                if (activeFocus) {
                    root.ensureContentVisibleRequested(Qt.rect(x, y, width, height))
                }
            }

            onEnsureContentVisibleRequested: {
                root.ensureContentVisibleRequested(contentRect)
            }
        }

        SeparatorLine {
            visible: uiColorsSection.visible
        }

        UiColorsSection {
            id: uiColorsSection

            width: content.width

            visible: appearanceModel.highContrastEnabled

            navigation.section: root.navigationSection
            //! NOTE: 3 because ThemesSection have two panels
            navigation.order: root.navigationOrderStart + 3

            onColorChangeRequested: {
                appearanceModel.setNewColor(newColor, propertyType)
            }

            onFocusChanged: {
                if (activeFocus) {
                    root.ensureContentVisibleRequested(Qt.rect(x, y, width, height))
                }
            }
        }

        SeparatorLine {}

        UiFontSection {
            allFonts: appearanceModel.allFonts()
            currentFontIndex: appearanceModel.currentFontIndex
            bodyTextSize: appearanceModel.bodyTextSize

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 4

            onFontChangeRequested: {
                appearanceModel.currentFontIndex = newFontIndex
            }

            onBodyTextSizeChangeRequested: {
                appearanceModel.bodyTextSize = newBodyTextSize
            }

            onFocusChanged: {
                if (activeFocus) {
                    root.ensureContentVisibleRequested(Qt.rect(x, y, width, height))
                }
            }
        }

        SeparatorLine {}

        ColorAndWallpaperSection {
            width: parent.width

            title: qsTrc("appshell", "Background")
            wallpaperDialogTitle: qsTrc("appshell", "Choose background wallpaper")
            useColor: appearanceModel.backgroundUseColor
            color: appearanceModel.backgroundColor
            wallpaperPath: appearanceModel.backgroundWallpaperPath
            wallpapersDir: appearanceModel.wallpapersDir()
            wallpaperFilter: appearanceModel.wallpaperPathFilter()

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 5

            onUseColorChangeRequested: {
                appearanceModel.backgroundUseColor = newValue
            }

            onColorChangeRequested: {
                appearanceModel.backgroundColor = newColor
            }

            onWallpaperPathChangeRequested: {
                appearanceModel.backgroundWallpaperPath = newWallpaperPath
            }

            onFocusChanged: {
                if (activeFocus) {
                    root.ensureContentVisibleRequested(Qt.rect(x, y, width, height))
                }
            }
        }

        SeparatorLine {}

        ColorAndWallpaperSection {
            width: parent.width

            title: qsTrc("appshell", "Paper")
            wallpaperDialogTitle: qsTrc("appshell", "Choose Notepaper")
            useColor: appearanceModel.foregroundUseColor
            color: appearanceModel.foregroundColor
            wallpaperPath: appearanceModel.foregroundWallpaperPath
            wallpapersDir: appearanceModel.wallpapersDir()
            wallpaperFilter: appearanceModel.wallpaperPathFilter()

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 6

            onUseColorChangeRequested: {
                appearanceModel.foregroundUseColor = newValue
            }

            onColorChangeRequested: {
                appearanceModel.foregroundColor = newColor
            }

            onWallpaperPathChangeRequested: {
                appearanceModel.foregroundWallpaperPath = newWallpaperPath
            }

            onFocusChanged: {
                if (activeFocus) {
                    root.ensureContentVisibleRequested(Qt.rect(x, y, width, height))
                }
            }
        }

        ResetThemeButtonSection {
            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 7

            onResetThemeToDefaultRequested: {
                appearanceModel.resetThemeToDefault()
            }

            onFocusChanged: {
                if (activeFocus) {
                    root.ensureContentVisibleRequested(Qt.rect(x, y, width, height))
                }
            }
        }
    }
}
